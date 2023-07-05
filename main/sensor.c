/*
 * sensor.c
 *
 *  Created on: 30 cze 2023
 *      Author: kkohut
 */
#include "sensor.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "vl53l1_api.h"

#define DEBUG (0u)

#if DEBUG
#define DEBUG_PRINT(fmt, args...) printf(fmt, ##args)
#else
#define DEBUG_PRINT(fmt, args...) /* nic nie robi */
#endif

#define GPIO_INPUT_IO_14     14
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_14)
#define ESP_INTR_FLAG_DEFAULT 0

#define VL53L1_WRONG_INDEX 	((VL53L1_Error)  100)

#define MAX_SENSORS 2u // Maksymalna liczba sensorów

#define MAX_EVT_QUEUE_COUNT 5u
#define MAX_RESULT_QUEUE_COUNT 5u


xQueueHandle gpio_evt_queue = NULL;


typedef struct {
	Sensor_pins_t pins;
    TaskHandle_t sensor_task_handle;
    VL53L1_Dev_t dev;
    QueueHandle_t gpio_evt_queue;
    QueueHandle_t result_queue;
} Sensor_t;

volatile Sensor_t sensors[MAX_SENSORS] = {0};;

static void IRAM_ATTR gpio_isr_handler(void* arg);
void sensor_task(void *pvParameters);
VL53L1_Error sensor_data_init(Sensor_t* sensor);
Sensor_t* get_sensor(int sensor_index);

VL53L1_Error sensor_init(int sensor_index, Sensor_pins_t pins, int i2c_address) {

	char sensor_task_name[16];

    // Sprawdź, czy indeks sensora jest w zakresie
	if (sensor_index < 0 || sensor_index >= MAX_SENSORS) {
		// Zwróć błąd, jeśli indeks sensora jest poza zakresem
		return VL53L1_WRONG_INDEX;
	}

	// Pobierz odpowiednią strukturę dla sensora
	Sensor_t* sensor = &sensors[sensor_index];

    // Inicjalizacja struktury
    sensor->pins.interrupt_pin = pins.interrupt_pin;
    sensor->pins.power_pin = pins.power_pin;
    sensor->pins.xshut_pin = pins.xshut_pin;

    // Ustawienie pinów
	gpio_set_direction(sensor->pins.interrupt_pin, GPIO_MODE_INPUT);

	gpio_set_direction(sensor->pins.power_pin, GPIO_MODE_OUTPUT);
	gpio_set_level(sensor->pins.power_pin, 0);

	gpio_set_direction(sensor->pins.xshut_pin, GPIO_MODE_OUTPUT);
	gpio_set_level(sensor->pins.power_pin, 0);

	// Konfiguracja przerwania
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	io_conf.pin_bit_mask = 1ULL << sensor->pins.interrupt_pin;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);

	// Utworzenie kolejki
	sensor->gpio_evt_queue = xQueueCreate(MAX_EVT_QUEUE_COUNT, sizeof(uint32_t));
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(sensor->pins.interrupt_pin, gpio_isr_handler, (void*) sensor->pins.interrupt_pin);

	// Utworzenie kolejki dla wyników pomiarów
	sensor->result_queue = xQueueCreate(MAX_RESULT_QUEUE_COUNT, sizeof(int16_t));

	// Przypisanie adresu i2c
	sensor->dev.i2c_slave_address = i2c_address;

    // Tworzenie tasku
	sprintf(sensor_task_name, "sensor_task_%d", sensor_index);
	xTaskCreate(sensor_task, sensor_task_name, 4096, sensor, 5, &sensor->sensor_task_handle);

    return VL53L1_ERROR_NONE;
}

VL53L1_Error sensor_data_init(Sensor_t* sensor)
{
	VL53L1_Error status = VL53L1_ERROR_NONE;

	VL53L1_Dev_t dev;
	dev.i2c_slave_address = 0x52;




	gpio_set_level(sensor->pins.power_pin, 1);
	vTaskDelay(pdMS_TO_TICKS(100));

	gpio_set_level(sensor->pins.xshut_pin, 1);
	vTaskDelay(pdMS_TO_TICKS(100));
	//ustaw nowy adres
	status = VL53L1_SetDeviceAddress(&dev, sensor->dev.i2c_slave_address);
	if (status != VL53L1_ERROR_NONE) {
		return status;
	}

	// Wywołanie funkcji API sensora
	status = VL53L1_DataInit(&sensor->dev);
	if (status != VL53L1_ERROR_NONE) {
		return status;
	}

	status = VL53L1_StaticInit(&sensor->dev);
	if (status != VL53L1_ERROR_NONE) {
		return status;
	}

	status = VL53L1_SetDistanceMode(&sensor->dev, VL53L1_DISTANCEMODE_LONG);
	if (status != VL53L1_ERROR_NONE) {
		return status;
	}

	status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(&sensor->dev, 90000);
	if (status != VL53L1_ERROR_NONE) {
		return status;
	}

	status = VL53L1_SetInterMeasurementPeriodMilliSeconds(&sensor->dev, 100);
	if (status != VL53L1_ERROR_NONE) {
		return status;
	}

	status = VL53L1_StartMeasurement(&sensor->dev);
	if (status != VL53L1_ERROR_NONE) {
		return status;
	}
	return status;
}

// Funkcja obsługi przerwania
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    // 'arg' to numer pinu przekazany podczas konfiguracji obsługi przerwania
    uint32_t gpio_num = (uint32_t) arg;

    // Znajdujemy odpowiednią strukturę sensora
    Sensor_t* sensor = NULL;
    for (int i = 0; i < MAX_SENSORS; ++i) {
        if (sensors[i].pins.interrupt_pin == gpio_num) {
            sensor = &sensors[i];
            break;
        }
    }

    if (sensor == NULL) {
        return; // Nie znaleziono odpowiadającego sensora, zakończ obsługę przerwania
    }

    // Tworzymy zmienną do przechowywania wyniku funkcji xQueueSendFromISR
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Dodajemy element do kolejki z obsługi przerwania.
    // xQueueSendFromISR jest specjalną wersją funkcji xQueueSend przeznaczoną do użytku w ISR.
    xQueueSendFromISR(sensor->gpio_evt_queue, &gpio_num, &xHigherPriorityTaskWoken);

    // Jeśli xHigherPriorityTaskWoken zostało ustawione na pdTRUE wewnątrz xQueueSendFromISR,
    // oznacza to, że zadanie o wyższym priorytecie jest gotowe do uruchomienia, a więc planista powinien
    // zostać zawiadomiony o zmianie kontekstu. W tym przypadku, ponieważ jesteśmy w ISR, używamy
    // makra portYIELD_FROM_ISR, które będzie miało efekt tylko wtedy, gdy wyjdziemy z ISR.

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

int16_t get_distance(int sensor_index)
{
    Sensor_t* sensor = get_sensor(sensor_index);
    int16_t s16_Distance;

    if (sensor == NULL) {
        // Zwróć pusty wynik, jeśli nie ma takiego sensora
        return (int16_t) {0};
    }
    xQueueReceive(sensor->result_queue, &s16_Distance, portMAX_DELAY);
    return s16_Distance;
}

Sensor_t* get_sensor(int sensor_index) {
    // Sprawdzanie, czy indeks jest w granicach tablicy
    if (sensor_index < 0 || sensor_index >= MAX_SENSORS) {
        // Zwróć NULL, jeśli indeks jest poza zakresem
        return NULL;
    }

    // Zwróć wskaźnik do sensora
    return &sensors[sensor_index];
}

void sensor_task(void* param) {
    Sensor_t* sensor = (Sensor_t*)param;
    VL53L1_Error status;
    VL53L1_RangingMeasurementData_t RangingData;
    uint32_t io_num;
	int16_t range = 4000u;

    for(;;) {
        // Proces inicjalizacji
    	DEBUG_PRINT("Start init!\n");
        status = sensor_data_init(sensor);
        if (status != VL53L1_ERROR_NONE) {

        	vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Proces odczytu danych
        for(;;) {
            // Czekaj na przerwanie
            if (xQueueReceive(sensor->gpio_evt_queue, &io_num, portMAX_DELAY)) {

                status = VL53L1_GetRangingMeasurementData(&sensor->dev, &RangingData);

                switch (RangingData.RangeStatus) {
                    case 0:
                        // Dodaj dane do kolejki
                        xQueueOverwrite(sensor->result_queue, &RangingData.RangeMilliMeter);

                        DEBUG_PRINT("Range: %d\n", RangingData.RangeMilliMeter);
                        break;
                    case 2:
                    	//zwraca dwa gdy obiekt jest dalej niz zasieg
                    	xQueueOverwrite(sensor->result_queue, &range);
                    	DEBUG_PRINT("Range: %d\n", 4000u);
                    	break;
                    default:
                        DEBUG_PRINT("Error: %d\n", RangingData.RangeStatus);
                        break;
                }

                VL53L1_ClearInterruptAndStartMeasurement(&sensor->dev);
            }
        }
    }
}




