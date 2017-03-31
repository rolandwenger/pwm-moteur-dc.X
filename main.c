#include <xc.h>
#include "test.h"

/**
 * Bits de configuration:
 */
#pragma config FOSC = INTIO67   // Osc. interne, A6 et A7 comme IO.
#pragma config IESO = OFF       // Pas d'osc. au démarrage.
#pragma config FCMEN = OFF      // Pas de monitorage de l'oscillateur.

// Nécessaires pour ICSP / ICD:
#pragma config MCLRE = EXTMCLR  // RE3 est actif comme master reset.
#pragma config WDTEN = OFF      // Watchdog inactif.
#pragma config LVP = OFF        // Single Supply Enable bits off.

typedef enum {
    AVANT = 0b01,
    ARRIERE = 0b10
} Direction;

/**
 * Indique la direction correspondante à la valeur du potentiomètre.
 * @param v Valeur du potentiomètre.
 * @return AVANT ou ARRIERE.
 */
Direction conversionDirection(unsigned char v) {
    
    if (v == 127){ // le potentiomètre est a 50%
        return 0; // les 2 sorties sont à 0
        
    } else if (v < 127){
        //plus petit que 50% donc marche arrière
        return ARRIERE;
    } else {
        //plus grand que 50% donc marche avant
       return AVANT; 
    }  
}

/**
 * Indique le cycle de travail PWM correspondant à la valeur du potentiomètre.
 * @param v Valeur du potentiomètre.
 * @return Cycle de travail du PWM.
 */
unsigned char conversionMagnitude(unsigned char v) {
     int magnitude; // initialisation d'un variable entière pour nos calculs
    
    if(v<128) // si le potentiomètre est en dessous de 50%
    {
        magnitude = 254-2*v;
    }
    else// si le potentiomètre est en dessus de 50%
    {
        magnitude = (v-128)*2;
    }
    
    return magnitude; // retourner la valeur calculée
 }

#ifndef TEST

/**
 * Initialise le hardware.
 */
static void hardwareInitialise() {
   
    //configuration de l'entrée
    ANSELA=0;
    ANSELC=0;
    ANSELBbits.ANSB3 = 1; //configuration comme entrée analogique,
    TRISBbits.RB3=1;//configure le port RB3 comme entrée
    
    //INTCON2bits.RBPU=0; //active la résitance de tirage pas nécessaire car diviseur de tension 
    //WPUBbits.WPUB3=1;//active la résitance de tirage pour le port RB3 pas nécessaire car diviseur de tension
    
    //configuratio des sortie digitales
    //configure le port C comme sortie digitale
    TRISCbits.RC0=0;
    TRISCbits.RC1=0;
           
       // Activer les interruptions:
    RCONbits.IPEN = 1;          // Active les niveaux d'interruptions.
    INTCONbits.GIEH = 1;        // Active les interruptions de haute priorité.
    INTCONbits.GIEL = 1;        // Active les interruptions de basse priorité.
	
	// Activer les interruptions du temporisateur 2:
    PIE1bits.TMR2IE = 1;      // Active les interruptions du TMR2.
    IPR1bits.TMR2IP = 1;     // Interruptions de haute priorité
    
    //Config Timer2
	T2CONbits.TMR2ON = 1; // Active le tmr2
    T2CONbits.TMR2ON = 1;      // Active le temporisateur.
    T2CONbits.T2OUTPS = 1001;   // division par 10 en sortie pour trm2if toute les 10ms
    T2CONbits.T2CKPS = 00;   // pas de division du prescaler d'entrée
	PR2 = 250; // Période du tmr2 à 1ms  1000/4 = 250
  
    // Activer le PWM sur CCP1
	CCPTMRS0bits.C1TSEL = 0; // CCP1 branché sur tmr2
	CCP1CONbits.CCP1M = 0xF; // Active le CCP1.
	TRISCbits.RC2 = 0; // Active la sortie du CCP1.
	
    //Conversion analogique digital
    ADCON0bits.ADON = 1;
    ADCON0bits.CHS = 1001; // configuration de la conversion analogique sur an9
    ADCON2bits.ADFM = 0;    // Les 8 bits plus signifiants sur ADRESH.
    
}

/**
 * Point d'entrée des interruptions.
 */
void low_priority interrupt interruptionsBassePriorite() {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;
        ADCON0bits.GO = 1;
    }
    
    if (PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        PORTC = conversionDirection(ADRESH);
        CCPR1L = conversionMagnitude(ADRESH);
    }
}

/**
 * Point d'entrée pour l'émetteur de radio contrôle.
 */
void main(void) {
    hardwareInitialise();

    while(1);
}
#endif

#ifdef TEST
void testConversionMagnitude() {
    testeEgaliteEntiers("CM01", conversionMagnitude(0),   254);
    testeEgaliteEntiers("CM02", conversionMagnitude(1),   252);
    testeEgaliteEntiers("CM03", conversionMagnitude(2),   250);
    
    testeEgaliteEntiers("CM04", conversionMagnitude(125),   4);
    testeEgaliteEntiers("CM05", conversionMagnitude(126),   2);
    
    testeEgaliteEntiers("CM06", conversionMagnitude(127),   0);
    testeEgaliteEntiers("CM07", conversionMagnitude(128),   0);

    testeEgaliteEntiers("CM08", conversionMagnitude(129),   2);
    testeEgaliteEntiers("CM09", conversionMagnitude(130),   4);
    
    testeEgaliteEntiers("CM10", conversionMagnitude(253), 250);
    testeEgaliteEntiers("CM11", conversionMagnitude(254), 252);
    testeEgaliteEntiers("CM12", conversionMagnitude(255), 254);
}
void testConversionDirection() {
    testeEgaliteEntiers("CD01", conversionDirection(  0), ARRIERE);    
    testeEgaliteEntiers("CD02", conversionDirection(  1), ARRIERE);    
    testeEgaliteEntiers("CD03", conversionDirection(127), ARRIERE);    
    testeEgaliteEntiers("CD04", conversionDirection(128), AVANT);
    testeEgaliteEntiers("CD05", conversionDirection(129), AVANT);
    testeEgaliteEntiers("CD06", conversionDirection(255), AVANT);    
}
void main() {
    initialiseTests();
    testConversionMagnitude();
    testConversionDirection();
    finaliseTests();
    while(1);
}
#endif
