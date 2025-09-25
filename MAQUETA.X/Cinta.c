#include <xc.h>
#include <stdio.h>
#include "CONFI.h"
#include "lcd.h"
#define _XTAL_FREQ 4000000
#define PA PORTBbits.RB0 //Originalemnte PA Y PP VAN EN UN MISMO PIN    
#define SOPT PORTBbits.RB1
#define FCD PORTBbits.RB2
#define FCIZ PORTBbits.RB3
#define CEXT PORTBbits.RB4
#define CAP PORTBbits.RB5
#define OPT2 PORTBbits.RB6
#define IND PORTBbits.RB7

#define D90 PORTCbits.RC0
#define SPIN PORTCbits.RC1
#define FC1 PORTCbits.RC2
#define FC2 PORTCbits.RC6
#define ENTRE PORTCbits.RC7


#define SAC PORTEbits.RE0
#define MCT LATEbits.LATE1
#define SAR LATEbits.LATE2

#define SCAR LATAbits.LA0
#define DES LATAbits.LA1
#define PIN LATAbits.LA2
#define ROT LATAbits.LA3
#define GIZQ LATAbits.LA4
#define GDER LATAbits.LA5
#define VENT LATAbits.LA6

#define PZA0 LATDbits.LATD0
#define PZA1 LATDbits.LATD1
//#define OUT  LATDbits.LATD2
int blanca=0;
int negra=0;
int metalica=0;
//Para mostrar una variable
void mostrar_variable(int x){
    char buffer[20];
     sprintf(buffer,"%d",x);
    Lcd_Write_String(buffer);
    return;
}
void init_ports(){
     ADCON1bits.PCFG=0x0F;// salida Digital
     TRISB=0XFF;
     TRISA=0X00;
     LATA=0X00;
     TRISD=0X00;
     TRISE=0X00;
     TRISEbits.RE0=1;
     LATE=0;
     TRISC=0XFF;
     return;
}
void init_int(){
    INTCONbits.GIEH=0;
    INTCONbits.GIEL=0;  //abajo interrupciones globales por ahora
    INTCONbits.RBIE=1;  // activamos el cambio en Rb
    INTCONbits.RBIF=0;  //Rb change flag abajo
    RCONbits.IPEN=1;    //Activamos las interrupciones de prioridad
    INTCON2bits.RBIP=0; //Cambio en RB como baja prioridad
}
void init_maqueta(){
    if(D90!=1){         //Si D90 no es 1, osea no esta en el estado esperado entra
        ROT=1;          // activamos el motor giratorio
        while(D90==0);  // mientras que D90 no este activado, va a girar hasta que D90 sea 1
        ROT=0;          // apaga cuando D90 sea 0
    }
    if(FCD!=1){
        SCAR=1;        //Activamos en contra de las manecillas del reloj y esperamos a que llegue a la posicion esperada
         while(FCD==0); 
         SCAR=0;        // apagamos luego de que llegue a donde es
    }
    if(FC2!=1){
        GDER=1;
        while(FC2==0);
        GDER=0;
    }
                    }
void mostrar_conteo(){
    Lcd_Write_String("Modulo");
    Lcd_Set_Cursor(2,1);
    Lcd_Write_String("Blancas=");
    mostrar_variable(blanca);
    Lcd_Set_Cursor(3,0);
    
}
//Interrupcion de alta prioridad
void __interrupt(high_priority) HighISR(void){
    
    return;
}

//Interrupcion de baja prioridad
void __interrupt(low_priority) LowISR(void){
     if (INTCONbits.RBIF){
       volatile unsigned char dummy = PORTB;    // obligamosa leer el puerto b
        INTCONbits.RBIF=0;                      // bajamos la bandera
        if(CAP==1){
            negra++;
            
        }
         if(IND==1){
            return;
        }
         if(OPT2==1){
            return;
        }
    }
    return;
}
//PROGRAMA PRINCIPAL
void main(void){
    int giro=0;
    init_int();
    init_ports();
    Lcd_Init();
    Lcd_Cmd(0x00);
    __delay_ms(10);
    Lcd_Cmd(0x0E);       // Ver cursor, luego de pruebas, borrar
    mostrar_conteo();
    init_maqueta();
    while(PA==0)  ;  // Esperamos el PA
    MCT=1;              // Encendemos el motor
    while(SOPT==0)  ;   //Esperamos el sensor optico
    MCT=0;
    SAR=1;              //Activamos girar con las agujas del reloj
    while(FCIZ==0);     //Hasta que el brazo efectivamente llegue a la izquierda
    SAR=0;          
    __delay_ms(1500);   //Esperamos 1.5sg para descender
    DES=1;
    __delay_ms(1000);   // encendemos la pinza
    PIN=1;
    while(SPIN==0);    // esperamos a que la pinza este cerrada
    DES=0;              // apagamos el des, para que el cilindro se extienda
    while(CEXT==0);    // esperamos que el cilindro este extendido
    SCAR=1;             // Activamos el giro en contra de las manecillas del reloj
    while(FCD==0) ;     //esperamos a que la pinza llegue a su destino
    DES=1;              //Bajamos el cilindro
    SCAR=0;
    __delay_ms(2000);
    PIN=0;              // Esperamos 2 segundos para solat la pieeza
    while(SPIN==1);
    DES=0;              // esperamos que spin, se desactive para subir el cilindro
    while(CEXT==0);     //Esperamos a que el cilindro se extienda
    ROT=1;              //Encendemos la mesa rotatoria
    __delay_ms(50);
    while (giro < 2) {  //entramos esperando que se cumplan 180 grados
        while (D90==1); // esperamos que se desactive
        while(D90==0);  // esperamos que D90 se active
        giro++;         // contamos una, osea 90 grados
    }                   // saldra cuando gire 180 grados, osea que el conteo se haga dos veces
    ROT=0;
    GIZQ=1;            
    while(FC1==0);  //Activamos el giro a la izquiera y esperamos el FC1
    GIZQ=0;
    VENT=1;
    __delay_ms(2000);
    GDER=1;
    while(FC2==0);  //Activamos el giro a la derecha y esperamos el FC2
    GDER=0;
    __delay_ms(500);
    VENT=0;
    return;
}