#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include "tc27xc/IfxStm_reg.h"
#include "tc27xc/IfxStm_bf.h"
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"

#include "system_tc2x.h"
#include "interrupts.h"
#include "led.h"
#include "uart_int.h"

#include "asm_prototype.h"

#include "jansson.h"
#include "cJSON.h"

volatile uint32_t g_sys_ticks;
volatile bool g_regular_task_flag;

/* timer callback handler */
static void my_timer_handler(void)
{
	++g_sys_ticks;

	if(0==g_sys_ticks%(2*SYSTIME_CLOCK))
	{
		LEDTOGGLE(1);
	}
}

const uint32_t HAL_GetTick(void)
{
	return g_sys_ticks;
}

/* timer reload value (needed for subtick calculation) */
static unsigned int reload_value;

/* pointer to user specified timer callback function */
static TCF user_handler = (TCF)0;

/* timer interrupt routine */
static void tick_irq(int reload_value)
{
	Ifx_STM *StmBase = systime_GetStmBase();

	/* set new compare value */
	StmBase->CMP[0].U += (unsigned int)reload_value;
	if (user_handler)
	{
		user_handler();
	}
}

/* Initialise timer at rate <hz> */
void TimerInit(unsigned int hz)
{
	unsigned int CoreID = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;
	Ifx_STM *StmBase = systime_GetStmBase();
	unsigned int frequency = SYSTEM_GetStmClock();
	int irqId;

	reload_value = frequency / hz;

	switch (CoreID)
	{
	default :
	case 0 :
		irqId = SRC_ID_STM0SR0;
		break;
	}

	/* install handler for timer interrupt */
	InterruptInstall(irqId, tick_irq, SYSTIME_ISR_PRIO, (int)reload_value);

	/* reset interrupt flag */
	StmBase->ISCR.U = (IFX_STM_ISCR_CMP0IRR_MSK << IFX_STM_ISCR_CMP0IRR_OFF);
	/* prepare compare register */
	StmBase->CMP[0].U = StmBase->TIM0.U + reload_value;
	StmBase->CMCON.U  = 31;
	StmBase->ICR.B.CMP0EN = 1;
}

/* Install <handler> as timer callback function */
void TimerSetHandler(TCF handler)
{
	user_handler = handler;
}

static inline void flush_stdout(void)
{
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
	{
		;
	}
}


/* Used by some code below as an example datatype. */
typedef struct str_poi_record
{
	const char *precision;
	double lat;
	double lon;
	const char *address;
	const char *city;
	const char *state;
	const char *zip;
	const char *country;
}poi_record;

/* Our "days of the week" array: */
const char *str_weekdays[7] =
{
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday"
};

/* Our matrix: */
const int numbers[3][3] =
{
		{0, -1, 0},
		{1, 0, 0},
		{0 ,0, 1}
};

/* Our "gallery" item: */
const int ids[4] = { 116, 943, 234, 38793 };
/* Our array of "records": */
const poi_record fields[2] =
{
		{
				"zip",
				37.7668,
				-1.223959e+2,
				"",
				"SAN FRANCISCO",
				"CA",
				"94107",
				"US"
		},
		{
				"zip",
				37.371991,
				-1.22026e+2,
				"",
				"SUNNYVALE",
				"CA",
				"94085",
				"US"
		}
};

void test_jansson(void)
{
	printf("Jansson Version:%s\n", JANSSON_VERSION);

	json_t* root;
	char* str_dump;

	//Object Video Creating Test
	{
		json_t* fmt;
		root = json_object();
		json_object_set_new(root, "name", json_string("Jack (\"Bee\") Nimble"));
		json_object_set_new(root, "format", fmt=json_object());
		json_object_set_new(fmt,  "type", json_string("rect"));
		json_object_set_new(fmt, "width", json_integer(1920));
		json_object_set_new(fmt, "height", json_integer(1080));
		json_object_set_new(fmt, "interlace", json_boolean(false));
		json_object_set_new(fmt, "frame rate", json_integer(24));

		str_dump = json_dumps( root, 0 );
		printf( str_dump );
		printf("\n");
		flush_stdout();
		free( str_dump );
		json_decref( root );
	}

	//1-Dim Array of String Test
	{
		root = json_array();

		for(size_t i=0; i<7; ++i )
		{
			json_array_append_new( root, json_string(str_weekdays[i]) );
		}

		str_dump = json_dumps( root, 0 );
		printf( str_dump );
		printf("\n");
		flush_stdout();
		free( str_dump );
		json_decref( root );
	}

	//2-Dim Array of Integer Test
	{
		root = json_array();

		for(size_t i=0; i<3; ++i )
		{
			json_t* jarr2 = json_array();
			for(size_t j=0; j<3; ++j )
			{
				json_array_append_new( jarr2, json_integer((json_int_t)numbers[i][j]));
			}
			json_array_append_new( root, jarr2);
		}

		str_dump = json_dumps( root, 0 );
		printf( str_dump );
		printf("\n");
		flush_stdout();
		free( str_dump );
		json_decref( root );
	}

	//Object Gallery Creating Test
	{
		json_t* img;
		json_t* thm;

		root = json_object();
		json_object_set_new(root, "Image", img=json_object());
		json_object_set_new(img, "Width", json_integer(800));
		json_object_set_new(img, "Height", json_integer(600));
		json_object_set_new(img,  "Title", json_string("View from 15th Floor"));
		json_object_set_new(img, "Thumbnail", thm=json_object());
		json_object_set_new(thm, "Url", json_string("http:/*www.example.com/image/481989943"));
		json_object_set_new(thm, "Height", json_integer(125));
		json_object_set_new(thm, "Width", json_string("100"));

		json_t* jarr = json_array();
		for(size_t i=0; i<sizeof(ids)/sizeof(ids[0]); ++i)
		{
			json_array_append_new( jarr, json_integer((json_int_t)ids[i]));
		}
		json_object_set_new(img, "IDs", jarr);

		str_dump = json_dumps( root, 0 );
		printf( str_dump );
		printf("\n");
		flush_stdout();
		free( str_dump );
		json_decref( root );
	}

	//Array of "records" Test
	{
		root = json_array();

		for (size_t i = 0; i < 2; i++)
		{
			json_t* item = json_object();

			json_object_set_new(item, "precision", json_string(fields[i].precision));
			json_object_set_new(item, "Latitude", json_real(fields[i].lat));
			json_object_set_new(item, "Longitude", json_real(fields[i].lon));
			json_object_set_new(item, "Address", json_string(fields[i].address));
			json_object_set_new(item, "City", json_string(fields[i].city));
			json_object_set_new(item, "State", json_string(fields[i].state));
			json_object_set_new(item, "Zip", json_string(fields[i].zip));
			json_object_set_new(item, "Country", json_string(fields[i].country));

			json_array_append_new( root, item);
		}

		str_dump = json_dumps( root, 0 );
		printf( str_dump );
		printf("\n");
		flush_stdout();
		free( str_dump );
		json_decref( root );
	}

	//null Test
	{
		root = json_object();
		volatile double zero = 0.0;

		json_object_set_new(root, "number", json_null());

		str_dump = json_dumps( root, 0 );
		printf( str_dump );
		printf("\n");
		flush_stdout();
		free( str_dump );
		json_decref( root );
	}
}


/* Create a bunch of objects as demonstration. */
static int print_preallocated(cJSON *root)
{
	/* declarations */
	char *out = NULL;
	char *buf = NULL;
	char *buf_fail = NULL;
	size_t len = 0;
	size_t len_fail = 0;

	/* formatted print */
	out = cJSON_Print(root);

	/* create buffer to succeed */
	/* the extra 5 bytes are because of inaccuracies when reserving memory */
	len = strlen(out) + 5;
	buf = (char*)malloc(len);
	if (buf == NULL)
	{
		printf("Failed to allocate memory.\n");
		exit(1);
	}

	/* create buffer to fail */
	len_fail = strlen(out);
	buf_fail = (char*)malloc(len_fail);
	if (buf_fail == NULL)
	{
		printf("Failed to allocate memory.\n");
		exit(1);
	}

	/* Print to buffer */
	if (!cJSON_PrintPreallocated(root, buf, (int)len, 1)) {
		printf("cJSON_PrintPreallocated failed!\n");
		if (strcmp(out, buf) != 0) {
			printf("cJSON_PrintPreallocated not the same as cJSON_Print!\n");
			printf("cJSON_Print result:\n%s\n", out);
			printf("cJSON_PrintPreallocated result:\n%s\n", buf);
		}
		free(out);
		free(buf_fail);
		free(buf);
		return -1;
	}

	/* success */
	printf("%s\n", buf);

	/* force it to fail */
	if (cJSON_PrintPreallocated(root, buf_fail, (int)len_fail, 1)) {
		printf("cJSON_PrintPreallocated failed to show error with insufficient memory!\n");
		printf("cJSON_Print result:\n%s\n", out);
		printf("cJSON_PrintPreallocated result:\n%s\n", buf_fail);
		free(out);
		free(buf_fail);
		free(buf);
		return -1;
	}
	flush_stdout();

	free(out);
	free(buf_fail);
	free(buf);
	return 0;
}

/* Create a bunch of objects as demonstration. */
static void create_objects(void)
{
	/* declare a few. */
	cJSON *root = NULL;
	cJSON *fmt = NULL;
	cJSON *img = NULL;
	cJSON *thm = NULL;
	cJSON *fld = NULL;
	int i = 0;

	volatile double zero = 0.0;

	/* Here we construct some JSON standards, from the JSON site. */

	/* Our "Video" datatype: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
	cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
	cJSON_AddStringToObject(fmt, "type", "rect");
	cJSON_AddNumberToObject(fmt, "width", 1920);
	cJSON_AddNumberToObject(fmt, "height", 1080);
	cJSON_AddFalseToObject (fmt, "interlace");
	cJSON_AddNumberToObject(fmt, "frame rate", 24);

	/* Print to text */
	if (print_preallocated(root) != 0) {
		cJSON_Delete(root);
		exit(EXIT_FAILURE);
	}
	cJSON_Delete(root);

	/* Our "days of the week" array: */
	root = cJSON_CreateStringArray(str_weekdays, 7);

	if (print_preallocated(root) != 0) {
		cJSON_Delete(root);
		exit(EXIT_FAILURE);
	}
	cJSON_Delete(root);

	/* Our matrix: */
	root = cJSON_CreateArray();
	for (i = 0; i < 3; i++)
	{
		cJSON_AddItemToArray(root, cJSON_CreateIntArray(numbers[i], 3));
	}

	/* cJSON_ReplaceItemInArray(root, 1, cJSON_CreateString("Replacement")); */

	if (print_preallocated(root) != 0) {
		cJSON_Delete(root);
		exit(EXIT_FAILURE);
	}
	cJSON_Delete(root);

	/* Our "gallery" item: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "Image", img = cJSON_CreateObject());
	cJSON_AddNumberToObject(img, "Width", 800);
	cJSON_AddNumberToObject(img, "Height", 600);
	cJSON_AddStringToObject(img, "Title", "View from 15th Floor");
	cJSON_AddItemToObject(img, "Thumbnail", thm = cJSON_CreateObject());
	cJSON_AddStringToObject(thm, "Url", "http:/*www.example.com/image/481989943");
	cJSON_AddNumberToObject(thm, "Height", 125);
	cJSON_AddStringToObject(thm, "Width", "100");
	cJSON_AddItemToObject(img, "IDs", cJSON_CreateIntArray(ids, 4));

	if (print_preallocated(root) != 0) {
		cJSON_Delete(root);
		exit(EXIT_FAILURE);
	}
	cJSON_Delete(root);

	/* Our array of "records": */
	root = cJSON_CreateArray();
	for (i = 0; i < 2; i++)
	{
		cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
		cJSON_AddStringToObject(fld, "precision", fields[i].precision);
		cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
		cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
		cJSON_AddStringToObject(fld, "Address", fields[i].address);
		cJSON_AddStringToObject(fld, "City", fields[i].city);
		cJSON_AddStringToObject(fld, "State", fields[i].state);
		cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
		cJSON_AddStringToObject(fld, "Country", fields[i].country);
	}

	/* cJSON_ReplaceItemInObject(cJSON_GetArrayItem(root, 1), "City", cJSON_CreateIntArray(ids, 4)); */

	if (print_preallocated(root) != 0) {
		cJSON_Delete(root);
		exit(EXIT_FAILURE);
	}
	cJSON_Delete(root);

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "number", 1.0 / zero);

	if (print_preallocated(root) != 0) {
		cJSON_Delete(root);
		exit(EXIT_FAILURE);
	}
	cJSON_Delete(root);
}

void test_cJSON(void)
{
	printf("Version: %s\n", cJSON_Version());
	flush_stdout();

	/* Now some samplecode for building objects concisely: */
	create_objects();
}

int main(void)
{
	SYSTEM_Init();

	SYSTEM_EnaDisCache(1);

	TimerInit(SYSTIME_CLOCK);
	TimerSetHandler(my_timer_handler);

	_init_uart(BAUDRATE);

	InitLED();

	printf("Test cJSON CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d\n",
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			SYSTEM_GetStmClock()/1000000,
			SYSTEM_IsCacheEnabled());
	flush_stdout();

	printf("\n\n\n\n\n\n\n\n\n\n");
	flush_stdout();

	test_jansson();
	flush_stdout();

	test_cJSON();
	flush_stdout();

	printf("\n\n\n\n\n\n\n\n\n\n");
	flush_stdout();

	g_regular_task_flag = true;
	while(1)
	{
		if(0==g_sys_ticks%(20*SYSTIME_CLOCK))
		{
			g_regular_task_flag = true;
		}

		if(g_regular_task_flag)
		{
			g_regular_task_flag = false;

			LEDTOGGLE(0);

			printf("%s CPU:%u MHz,Sys:%u MHz, %u, CacheEn:%d\n",
					__TIME__,
					SYSTEM_GetCpuClock()/1000000,
					SYSTEM_GetSysClock()/1000000,
					HAL_GetTick(),
					SYSTEM_IsCacheEnabled());

			__asm__ volatile ("nop" ::: "memory");
			__asm volatile ("" : : : "memory");
			/* wait until sending has finished */
			while (_uart_sending())
				;
		}
	}

	return EXIT_SUCCESS;
}

