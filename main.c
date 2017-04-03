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
    // À implémenter.
    if(v<=127){ //i le potentiomètre est à moins de 50 %, le moteur tourne en arrière
        return ARRIERE;
    }
    else {      //Si le potentiomètre est à plus de 50 %, le moteur tourne en avant
    return AVANT;
    }
}

/**
 * Indique le cycle de travail PWM correspondant à la valeur du potentiomètre.
 * @param v Valeur du potentiomètre.
 * @return Cycle de travail du PWM.
 */
unsigned char conversionMagnitude(unsigned char v) {
    // À implémenter.
    int magnitude;
    
    if(v<128){
        magnitude=254-v*2;
    }
    else{
        magnitude=(v-128)*2;
    }
    return magnitude;
}

#ifndef TEST

/**
 * Initialise le hardware.
 */
static void hardwareInitialise() {
    // À implémenter.
    
    //configuration de l'entrée
    ANSELA=0;
    ANSELC=0;
    
    ANSELBbits.ANSB3 = 1; //configuration comme entrée analogique,
    TRISBbits.RB3=1;//configure le port RB3 comme entrée
    //INTCON2bits.RBPU=0; //active la résitance de tirage pas nécessaire car diviseur de tension 
    //WPUBbits.WPUB3=1;//active la résitance de tirage pour le port RB3 pas nécessaire car diviseur de tension
    
    //configuration des sorties digitales
   
    TRISCbits.RC0=0; //configure le port RC0 comme sortie digitale
    TRISCbits.RC1=0; //configure le port RC1 comme sortie digitale
    
    // configuration des interruptions:
    RCONbits.IPEN = 1; //Active les niveaux de priorité pour les interruptions
    INTCONbits.GIEH = 1; //Les interruptions de haute priorités sont activées
    INTCONbits.GIEL = 1; //Les interruptions de basse priorité sont activées

    // configuration du temporisateur 2
    T2CONbits.TMR2ON = 1;       // Active le temporisateur 2
    T2CONbits.T2CKPS = 00;       // Pas de diviseur de fréq. pour temporisateur 2 
    T2CONbits.T2OUTPS = 1001;   // divison de la fréquence de sortie par 10
    PR2 = 250;                   // Période du temporisateur 2: 250/(Fosc/4)=1 kHz

    // Prépare les interruptions de haute priorité temporisateur 2:
    PIE1bits.TMR2IE = 1;        // Active les interruptions.
    IPR1bits.TMR2IP = 1;        // En haute priorité.
    PIR1bits.TMR2IF = 0;        // Baisse le drapeau.
    
    // Active le PWM sur CCP1:
    CCPTMRS0bits.C1TSEL = 00;    // CCP1 branché sur tmr2
    CCP1CONbits.P1M = 00;       // Comportement comme générateur PWM simple. Sortie uniquement sur P1A 
    CCP1CONbits.CCP1M = 0xF;    // Active le CCP1
    TRISCbits.RC2=0; //configure le port RC1 comme sortie digitale
    
    // Configure le module A/D:

    ADCON0bits.ADON = 1;    // Allume le module A/D
    ADCON0bits.CHS = 9;     // Branche le convertisseur sur AN9
    ADCON2bits.ADFM = 1;    // Les 8 bits moins signifiants ...
                            // ... sont sur ADRESL
    ADCON2bits.ACQT = 3;    // Temps d'acquisition à 6 TAD.
    ADCON2bits.ADCS = 0;    // À 1MHz, le TAD est à 2us.
    
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
