/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LEDs
#define LED_PIO            PIOC
#define LED_PIO_ID         ID_PIOC
#define LED_PIO_IDX        8
#define LED_PIO_IDX_MASK   (1 << LED_PIO_IDX)

// LEDs 
#define LED1_PIO           PIOA                 
#define LED1_PIO_ID	       ID_PIOA              
#define LED1_PIO_IDX       0                    
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)   

// LEDs
#define LED3_PIO           PIOB
#define LED3_PIO_ID		   ID_PIOB
#define LED3_PIO_IDX       2
#define LED3_PIO_IDX_MASK  (1 << LED3_PIO_IDX)

// LEDs 
#define LED2_PIO           PIOC                 
#define LED2_PIO_ID		   ID_PIOC              
#define LED2_PIO_IDX       30                    
#define LED2_PIO_IDX_MASK  (1 << LED2_PIO_IDX)      

// Configuracoes do botao
#define BUT_PIO          PIOA
#define BUT_PIO_ID       ID_PIOA
#define BUT_PIO_IDX      11
#define BUT_PIO_IDX_MASK (1u << BUT_PIO_IDX)

// Configuracoes do botao
#define BUT1_PIO         PIOD
#define BUT1_PIO_ID      ID_PIOD
#define BUT1_PIO_IDX     28
#define BUT1_PIO_IDX_MASK (1u << BUT1_PIO_IDX) // esse já está pronto.

// Configuracoes do botao
#define BUT2_PIO         PIOC
#define BUT2_PIO_ID      ID_PIOC
#define BUT2_PIO_IDX     31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX) // esse já está pronto.

// Configuracoes do botao
#define BUT3_PIO         PIOA
#define BUT3_PIO_ID      ID_PIOA
#define BUT3_PIO_IDX     19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX) // esse já está pronto.

/*  Default pin configuration (no attribute). */
#define _PIO_DEFAULT             (0u << 0)
/*  The internal pin pull-up is active. */
#define _PIO_PULLUP              (1u << 0)
/*  The internal glitch filter is active. */
#define _PIO_DEGLITCH            (1u << 1)
/*  The internal debouncing filter is active. */
#define _PIO_DEBOUNCE            (1u << 3)

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);
void _pio_set(Pio *p_pio, const uint32_t ul_mask);
void _pio_clear(Pio *p_pio, const uint32_t ul_mask);
void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable);
void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute);
void _pio_set_output(Pio *p_pio, const uint32_t ul_mask,
const uint32_t ul_default_level, const uint32_t ul_multidrive_enable, const uint32_t ul_pull_up_enable);
uint32_t _pio_get(Pio *p_pio, const pio_type_t ul_type, const uint32_t ul_mask);
void _delay_ms(const uint32_t delay);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/
void _pio_set(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_SODR = ul_mask;
}

void _pio_clear(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_CODR = ul_mask;
}

void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask,
const uint32_t ul_pull_up_enable)
{
	/* Enable the pull-up(s) if necessary */
	if (ul_pull_up_enable) {
		p_pio->PIO_PUER = ul_mask;
		} else {
		p_pio->PIO_PUDR = ul_mask;
	}
}

void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute)
{
	_pio_pull_up(p_pio, ul_mask, ul_attribute);
	
	if (ul_attribute & (PIO_DEGLITCH || PIO_DEBOUNCE)){
		p_pio->PIO_IFER = ul_mask;
	}
	else{
		p_pio->PIO_IFDR = ul_mask;
	}
}

void _pio_set_output(Pio *p_pio, const uint32_t ul_mask,
const uint32_t ul_default_level,
const uint32_t ul_multidrive_enable,
const uint32_t ul_pull_up_enable)
{
	pio_disable_interrupt(p_pio, ul_mask);
	pio_pull_up(p_pio, ul_mask, ul_pull_up_enable);

	/* Enable multi-drive if necessary */
	if (ul_multidrive_enable) {
		p_pio->PIO_MDER = ul_mask;
		} else {
		p_pio->PIO_MDDR = ul_mask;
	}

	/* Set default value */
	if (ul_default_level) {
		p_pio->PIO_SODR = ul_mask;
		} else {
		p_pio->PIO_CODR = ul_mask;
	}

	/* Configure pin(s) as output(s) */
	p_pio->PIO_OER = ul_mask;
	p_pio->PIO_PER = ul_mask;
}

uint32_t _pio_get(Pio *p_pio, const pio_type_t ul_type,
const uint32_t ul_mask)
{
	uint32_t ul_reg;

	if ((ul_type == PIO_OUTPUT_0) || (ul_type == PIO_OUTPUT_1)) {
		ul_reg = p_pio->PIO_ODSR;
		} else {
		ul_reg = p_pio->PIO_PDSR;
	}

	if ((ul_reg & ul_mask) == 0) {
		return 0;
		} else {
		return 1;
	}
}

void _delay_ms(const uint32_t delay_ms) {
	for (int i = 0; i < (150000*delay_ms); i++) {
		asm("NOP");
	}
}

// Função de inicialização do uC
void init(void)
{
	// Initialize the board clock
	sysclk_init();
	
	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	 // Ativa PIOs
	 pmc_enable_periph_clk(LED_PIO_ID);
	 pmc_enable_periph_clk(LED1_PIO_ID);
	 pmc_enable_periph_clk(LED2_PIO_ID);
	 pmc_enable_periph_clk(LED3_PIO_ID);
	 
	 //Inicializa PC8 como entrada
	 _pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 1,0,0);
	 _pio_set_output(LED1_PIO, LED1_PIO_IDX_MASK, 1,0,0);
	 _pio_set_output(LED2_PIO, LED2_PIO_IDX_MASK, 1,0,0);
	 _pio_set_output(LED3_PIO, LED3_PIO_IDX_MASK, 1,0,0);
	 
	 // Inicializa PIO do botao
	 pmc_enable_periph_clk(BUT_PIO_ID);
	 pmc_enable_periph_clk(BUT1_PIO_ID);
	 pmc_enable_periph_clk(BUT2_PIO_ID);
	 pmc_enable_periph_clk(BUT3_PIO_ID);
	 
	 // Inicializa PIO do botao
	_pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT1_PIO, BUT1_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT2_PIO, BUT2_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT3_PIO, BUT3_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);

}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {
	if(!_pio_get(BUT_PIO, PIO_INPUT, BUT_PIO_IDX_MASK)) {
		for (int i=0; i<5; i++) {
			_pio_clear(LED_PIO, LED_PIO_IDX_MASK);
			_delay_ms(100);                         
			_pio_set(LED_PIO, LED_PIO_IDX_MASK);
			_delay_ms(100);                         			
		}
		
		}if(!_pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
		for (int i=0; i<5; i++) {
			_pio_clear(LED1_PIO, LED1_PIO_IDX_MASK);
			_delay_ms(100);                         
			_pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
			_delay_ms(100);                         						
		}
		
		}if(!_pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)) {
		for (int i=0; i<5; i++) {
			_pio_clear(LED2_PIO, LED2_PIO_IDX_MASK);
			_delay_ms(100);                        
			_pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
			_delay_ms(100);                         						
		}
		
		} if(!_pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK)) {
		for (int i=0; i<5; i++) {
			_pio_clear(LED3_PIO, LED3_PIO_IDX_MASK);
			_delay_ms(100);                         
			_pio_set(LED3_PIO, LED3_PIO_IDX_MASK);
			_delay_ms(100);                        						
		}
		}
	}
  return 0;
}
