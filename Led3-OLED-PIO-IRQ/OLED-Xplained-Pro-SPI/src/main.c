/************************************************************************
 * 5 semestre - Eng. da Computao - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Material:
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 *
 * Objetivo:
 *  - Demonstrar interrupção do PIO
 *
 * Periféricos:
 *  - PIO
 *  - PMC
 *
 * Log:
 *  - 10/2018: Criação
 ************************************************************************/

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Botão

#define BUT1_PIO         PIOD
#define BUT1_PIO_ID      ID_PIOD
#define BUT1_PIO_IDX     28
#define BUT1_PIO_IDX_MASK (1u << BUT1_PIO_IDX) 

#define BUT2_PIO         PIOC
#define BUT2_PIO_ID      ID_PIOC
#define BUT2_PIO_IDX     31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX) 

#define BUT3_PIO         PIOA
#define BUT3_PIO_ID      ID_PIOA
#define BUT3_PIO_IDX     19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX) 
/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/
volatile char but_flag1;
volatile char but_flag2;
volatile char but_flag3;
/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void io_init(void);
void pisca_led(int n, int t);

/************************************************************************/
/* handler / callbacks                                                  */
/************************************************************************/

/*
 * Exemplo de callback para o botao, sempre que acontecer
 * ira piscar o led por 5 vezes
 *
 * !! Isso é um exemplo ruim, nao deve ser feito na pratica, !!
 * !! pois nao se deve usar delays dentro de interrupcoes    !!
 */

void but_callback1(void)
{
	if (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
		but_flag1 = 1;
	}
	else {
		but_flag1 = 0;
	}
}

void but_callback2(void)
{
	if (!pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)) {
		but_flag2 = 1;
	}
	else {
		but_flag2 = 0;
	}
}

void but_callback3(void)
{
	if (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK)) {
		but_flag3 = 1;
	}
	else {
		but_flag3 = 0;
	}
}

/************************************************************************/
/* funções                                                              */
/************************************************************************/

// pisca led N vez no periodo T
void pisca_led(int n, int t){
	int anda = 10;
	for (int i=0;i<n;i++){
		if (but_flag2){
			pio_set(LED_PIO, LED_IDX_MASK);
			break;
		}
		else{
			pio_clear(LED_PIO, LED_IDX_MASK);
			delay_ms(t);
			pio_set(LED_PIO, LED_IDX_MASK);
			delay_ms(t);
			gfx_mono_draw_string("|", anda, 0, &sysfont);
			anda += 11;
		}
	}
}

// Inicializa botao SW0 do kit com interrupcao
void io_init(void)
{
	 // Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

	 // Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

	 // Configura PIO para lidar com o pino do botão como entrada
	 // com pull-up
	 pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	 pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);

	 pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	 pio_set_debounce_filter(BUT2_PIO, BUT2_PIO_IDX_MASK, 60);
	 
	 pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	 pio_set_debounce_filter(BUT3_PIO, BUT3_PIO_IDX_MASK, 60);
	 
	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but_callback1);

	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but_callback2);

	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but_callback3);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);

	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT2_PIO);

	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT3_PIO);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4);

	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 3);

	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4);

}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
void main(void)
{
	// Inicializa clock
	sysclk_init();
	int period = 500;
	board_init();
	sysclk_init();
	delay_init();
	io_init();
	
	// Init OLED
	gfx_mono_ssd1306_init();
	
	gfx_mono_draw_string("Delay", 10, 18, &sysfont);
		
	// super loop
	// aplicacoes embarcadas no devem sair do while(1).
	while(1)
  {
	  if (but_flag1){
		  period += 100;
		  pisca_led(10, period);
		  char str[128]; 
		  sprintf(str, "%d", period); 
		  pio_set(LED_PIO, LED_IDX_MASK);
		  gfx_mono_draw_string(str, 70, 18, &sysfont);
		  but_flag1 = 0;
	  }
	  
	  if (but_flag3){
		  period -= 100;
		  pisca_led(10, period);
		  char str[128]; 
		  sprintf(str, "%d", period); 
		  pio_set(LED_PIO, LED_IDX_MASK);
		  gfx_mono_draw_string(str, 70, 18, &sysfont);
		  but_flag3 = 0;
	  }
	  pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
  }
}
