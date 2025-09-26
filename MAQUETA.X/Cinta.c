#include <xc.h>
#include <stdio.h>
#include "CONFI.h"
#include "lcd.h"
#include <stdbool.h>
#define _XTAL_FREQ 1000000UL

#define PA PORTBbits.RB0 //Originalemnte PA Y PP VAN EN UN MISMO PIN
#define FCD PORTBbits.RB1
#define FCIZ PORTBbits.RB2
#define CEXT PORTBbits.RB3
#define SPIN PORTBbits.RB4
#define D90 PORTBbits.RB5
#define CAP PORTBbits.RB6
#define OPT2 PORTBbits.RB7

#define GIZQ LATCbits.LC0
#define GDER LATCbits.LC1
#define VENT LATCbits.LC6
#define SOPT PORTCbits.RC7

#define IND PORTAbits.RA1
#define FC1 PORTAbits.RA2
#define FC2 PORTAbits.RA3
#define DES LATAbits.LA4
#define PIN LATAbits.LA5
#define ROT LATAbits.LA6

#define MCT LATDbits.LATD0
#define MCTA LATDbits.LATD1
#define SAR LATDbits.LATD2
#define SCAR LATDbits.LATD3
#define RETRO PORTEbits.RE2

//#define OUT  LATDbits.LATD2
char blanca=0;
char negra=0;
char metalica=0;
char banderaN=0;
char banderaB=0;
char banderaM=0;
char escena=0;

void mostrar_variable(int x){
    char buffer[20];
     sprintf(buffer,"%d",x);
    Lcd_Write_String(buffer);
    return;
}


void init_ports(){
     ADCON1bits.PCFG=0x0F;// salida Digital
     TRISB=0XFF;
     TRISA=0b00001111;
     LATA=0X00;
     TRISD=0X00;
     TRISE=0X00;
     LATE=0;
     TRISC=0X80;
     LATA=0;
     LATE=0;
     LATB=0;
     LATC=0;
     LATD=0;
     return;
}
void init_int(){
    INTCONbits.GIEH=0;
    INTCONbits.GIEL=0;  //abajo interrupciones globales por ahora
    INTCONbits.RBIE=0;  // desactivamos el cambio en Rb
    INTCONbits.RBIF=0;  //Rb change flag abajo
    RCONbits.IPEN=0;    //DesActivamos las interrupciones de prioridad
    INTCON2bits.RBIP=0; //Cambio en RB como baja prioridad
}

void init_maqueta(){
    
    if(D90!=0){         //Si D90 no es 1, osea no esta en el estado esperado entra
        ROT=1;          // activamos el motor giratorio
        while(D90==1);  // mientras que D90 no este activado, va a girar hasta que D90 sea 1
        ROT=0;          // apaga cuando D90 sea 0
    }
    if(FCD!=0){
        SCAR=1;        //Activamos en contra de las manecillas del reloj y esperamos a que llegue a la posicion esperada
         while(FCD==1); 
         SCAR=0;        // apagamos luego de que llegue a donde es
    }
    if(FC2!=0){
        GDER=1;
        while(FC2==1);
        GDER=0;
    }
}
void mostrar_conteo(){
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    if(escena==0){
        Lcd_Write_String("Configurando");
    }
    if(escena==1){
    Lcd_Write_String("Inicializando");
    }
    if(escena==2){
    Lcd_Write_String("START");
    }
    if(escena==3){
    Lcd_Write_String("Pieza en cinta");
    }
    if(escena==4){
    Lcd_Write_String("Agarrando pieza");
    }
    if(escena==5){
    Lcd_Write_String("Plato rotativo");
    }
    if(escena==6){
    Lcd_Write_String("Entregando");
    }
    Lcd_Set_Cursor(2,1);
    if (escena>5){
    if(banderaN==1){
    Lcd_Write_String("NEGRA");
    }
    if(banderaM==1){
     Lcd_Write_String("METALICA");
    }
    if ((banderaM==0)&&(banderaN==0)) {
      Lcd_Write_String("BLANCA");
    }
    }
    else{
       Lcd_Write_String("NADA");
    }
    Lcd_Cmd(0x09);
    __delay_ms(10);
    Lcd_Cmd(0x00);
    Lcd_Write_String("M:");
    mostrar_variable(metalica);
    Lcd_Write_String(" B:");
    mostrar_variable(blanca);
    Lcd_Write_String(" N:");
    mostrar_variable(negra);
    
}
//Interrupcion de STOP
void __interrupt(high_priority) Stop(void){
    RETRO = PA;
while(PA==0){
    RETRO = PA;
};  
RETRO = PA;
INTCONbits.INT0IF = 0;
}

//PROGRAMA PRINCIPAL
void main(void){
    int giro=0;
    //init_int();
    init_ports();
    Lcd_Init();
    Lcd_Cmd(0x00);
    __delay_ms(10);
    Lcd_Cmd(0x0E);       // Ver cursor, luego de pruebas, borrar
    Lcd_Set_Cursor(1,1);
    mostrar_conteo();
    __delay_ms(500);
    escena=1; //Inicializando
    mostrar_conteo();
    init_maqueta();
    __delay_ms(1000);
    escena=2; //START
    mostrar_conteo();
    while (1){ 
    while(PA==0);  // Esperamos el PA
    RETRO = PA;
    RCONbits.IPEN = 1;
    INTCON2bits.INTEDG0 = 0;
    INTCONbits.GIEH = 1;
    INTCONbits.INT0F = 1;
    escena=3; //CINTA
    mostrar_conteo();
    MCT=1;              // Encendemos el motor
    while(SOPT==1)  ;   //Esperamos el sensor optico
    MCT=0;
    escena=4; //PINZA
    mostrar_conteo();
    SAR=1;              //Activamos girar con las agujas del reloj
    while(FCIZ==1);     //Hasta que el brazo efectivamente llegue a la izquierda
    SAR=0;          
    __delay_ms(1000);   //Esperamos 1.5sg para descender
    __delay_ms(500);
    DES=1;
    __delay_ms(1000);   // encendemos la pinza
    PIN=1;
    while(SPIN==1);    // esperamos a que la pinza este cerrada
    DES=0;             // apagamos el des, para que el cilindro se extienda
    while(CEXT==1);    // esperamos que el cilindro este extendido
    SCAR=1;             // Activamos el giro en contra de las manecillas del reloj
    while(FCD==1) ;     //esperamos a que la pinza llegue a su destino
    DES=1;              //Bajamos el cilindro
    __delay_ms(1000);   //Esperamos 1.5sg para descender
    SCAR=0;
    __delay_ms(1000);
    __delay_ms(1000);
    PIN=0;              // Esperamos 2 segundos para soltar la pieza
    while(SPIN==1);
    escena=5; //PLATO
    mostrar_conteo();
    DES=0;              // esperamos que spin, se desactive para subir el cilindro
    while(CEXT==1);     //Esperamos a que el cilindro se extienda
    ROT=1;              //Encendemos la mesa rotatoria
    while (giro < 2) {  //entramos esperando que se cumplan 180 grados
        if(IND==0){
            banderaM=1;
         };       
        while (D90==0); // esperamos que se desactive
        while(D90==1){
             if(IND==0){
            banderaM=1;
         };       
        if((OPT2==1)){
            banderaN=1;
        };
        };  // esperamos que D90 se active
        giro++;         // contamos una, osea 90 grados
    };                   // saldra cuando gire 180 grados, osea que el conteo se haga dos veces
    if(banderaN==1){
        negra++;
    };
    if(banderaM==1){
        metalica++;
    };
    if((banderaM==0)&&(banderaN==0)){
        blanca++;
    };
    ROT=0;
    escena=6;
    mostrar_conteo();
    GIZQ=1;            
    while(FC1==1);  //Activamos el giro a la izquiera y esperamos el FC1
    GIZQ=0;
    mostrar_conteo();
    VENT=1;
    __delay_ms(1000);
    __delay_ms(1000);
    GDER=1;
    while(FC2==1);  //Activamos el giro a la derecha y esperamos el FC2
    GDER=0;
    __delay_ms(500);
    VENT=0;
    banderaM=0;
    banderaN=0;
    banderaB=0;
    giro= 0;
    };
    return;
}