#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//na pocetku glave 9 u knjizi iz RiPa i principa su ti objasnjeni za sta koji sluzi

int RAM[4096][16];
int PC[12];
int AC[16], E;
int I, OPR[3], MAR[12]; //ovo se koristi da se u stvari vidi koja je naredba u pitanju
// slike sa stranica 237 i 238 to iscitaj i pokusaj da shvatis kako funkcionisu adresiranja

int MBR[16];
int SC[2]; //generator taktova
int S = 1; //ovo je prekidac za dozvolu rada racunara, 1 da je upaljen 0 da ugasis racunar.. (kasnije ces vidjeti HLT naredbu, to se ne radi rucno ovako)
int F, R;
int t[4]; //taktovi
int q[8]; //za q od 0 do 7 su u pitanju memorijske, za q[8] registarske koje posle po mbr-u citas (registarske ne mogu biti indirektne i to zapamti!!!)
//q su u stvari izlazi kad OPR provuces kroz dec3x8
int c[4];  //ciklusi.. imas 4 ciklusa fetch,indirect,execute i interrupt(njega ne radimo u ovom projektu)
int F, R; //F i R su ti za citanje koji je ciklus u pitanju, to vidi na 240,241 str
//F=0, R=0 --- > c0=1; c1=0; c2=0; c3=0 fetch ciklus
//F=0, R=1 --- > c0=0; c1=1; c2=0; c3=0 indirect
//F=1, R=0 --- > c0=0; c1=0; c2=1; c3=0 execute
//F=1, R=1 --- > c0=0; c1=0; c2=0; c3=1 interrupt


void timeDecoder2x4(int x [2], int y [4], int e){
    y[0] = !x[0] && !x[1] && e;
    y[1] = !x[0] && x[1] && e;
    y[2] = x[0] && !x[1] && e;
    y[3] = x[0] && x[1] && e;
}//str 241, kroz njega ces npr F i R da provuces da bi se ciklusi smjenjivali
void operationDecoder3x8(int x [3], int y [8], int e){
    y[0] = !x[0] && !x[1] && !x[2] && e;
    y[1] = !x[0] && !x[1] && x[2] && e;
    y[2] = !x[0] && x[1] && !x[2] && e;
    y[3] = !x[0] && x[1] && x[2] && e;
    y[4] = x[0] && !x[1] && !x[2] && e;
    y[5] = x[0] && !x[1] && x[2] && e;
    y[6] = x[0] && x[1] && !x[2] && e;
    y[7] = x[0] && x[1] && x[2] && e;
}
int allZeros(int x [], int n){
    for(int i = 0; i < n; i++){
        if(x[i] != 0) return 0;
    }
    return 1;
}
void increaseRegister(int x[], int n){
    int q = 1;

    for(int i = n - 1; i >= 0; i--){
        int qn = x[i] && q;
        x[i] = q ^ x[i];
        q = qn;
    }
}
void printRegister(int *a, int n){
    for(int i = 0; i < n; i++)
        printf("%d", a[i]);
    printf("\n");
}
int binToDec16(int x[16]){

    int base = 1;
    int dec = 0;
    for(int i = 15; i>=0; i--){
        dec += x[i]*base;
        base *= 2;
    }
    return dec;
}
int binToDec12(int x[12]){

    int base = 1;
    int dec = 0;
    for(int i = 11; i >= 0; i--){
        dec += x[i] * base;
        base *= 2;
    }
    return dec;
}
void decToBin(int x, int niz[], int n){
    if(x<0) x*=-1;

    for(int i = 0; i < n; i++) {
        int stepenDvojke = pow(2, n - 1 - i);
        niz[i] = x / stepenDvojke;
        x = x - niz[i] * stepenDvojke;
    }
}
void sabirac(int a[] , int b[] , int *p ,  int n){
    int q = 0 ;
    int c[n];
    for(int i = 0; i < n ; i++){
        c[i] = a[i];
    }
    for(int i = n-1; i >= 0 ; i--){
         a[i] = (c[i] ^ b[i]) ^ q;
         q = (c[i] & b[i]) | ((c[i] ^ b[i]) & q);
    }
    *p = q;
}
void drugiKomplement(int a[], int n){

    for(int i = 0; i < n; i++){
        if(a[i] == 0)
            a[i] = 1;
        else
            a[i] = 0;
    }

    increaseRegister(a, n);
}
void copyRegister(int arr1[], int arr2[], int n){
    for(int i=0; i < n; i++){
        arr2[i] = arr1[i];
    }
}



//radimo sa dva tipa naredbi
//memorijske i registaarske
//npr prva memorijska je and i treba da se izvrsi operacija logicko i izmedju memorijske rijeci i akumulatora
//sad ti je bitno da vidis u kojem taktu se sta radi
//za svaku memorijsku naredbu taktovi 0 1 i 3 su nam isti, u t2 se radi taj "glavni" zadatak odnosno ono sto ti ime operacije govori
//sve se to radi u execute ciklusu


void and(){
    if(t[0]){
        copyRegister(MBR + 4, MAR, 12);
        //dio 5-16 registra MBR sadrzi memorisjku rijec (adresni dio naredbe)
        //tu memorijsku rijec prepisujemo u MAR (adresni registar)
        //str 245
        return;
    }
    if(t[1]){
        copyRegister(RAM[binToDec12(MAR)], MBR, 16);
        //sad je potrebno  da se ta operacija procita
        //upisana je u ramu, odatle je cita i pise u mbr
        return;
    }
    if(t[2]){
    for(int i=0; i<16; i++){
        AC[i] = AC[i] && MBR[i];
        return; //ovo dodje taj "glavni" kod naredbe,  logicko i
        }
    }

    if(t[3]){
        F = 0; //postavi F na 0 da bi osigurali prelazak u naredni fetch ciklus
        return;
    }
}
void add(){ //uveca akumulator za memorijsku rijec
    if(t[0]){
        copyRegister(MBR + 4, MAR, 12);
        return;
    }
    if(t[1]){
        copyRegister(RAM[binToDec12(MAR)], MBR, 16);
        return;
    }
    if(t[2]){
        sabirac(AC, MBR, &E, 16);
        return;
    }
    if(t[3]){
        F = 0;
        return;
    }
}
void lda(){ //donesi iz memorije u akumulator
    if(t[0]){
        copyRegister(MBR + 4, MAR, 12);
        return;
    }
    if(t[1]){
        copyRegister(RAM[binToDec12(MAR)], MBR, 16);
        return;
    }
    if(t[2]){
        copyRegister(MBR, AC, 16);
        return;
    }
    if(t[3]){
        F = 0;
        return;
    }
}
void sta(){ //iz akumulatora u memoriju
    if(t[0]){
        copyRegister(MBR + 4, MAR, 12);
        return;
    }
    if(t[1]){
        copyRegister(RAM[binToDec12(MAR)], MBR, 16);
        return;
    }
    if(t[2]){
        copyRegister(AC, RAM[binToDec12(MAR)], 16);
        return;
    }
    if(t[3]){
        F = 0;
        return;
    }
}
void bun(){ //bezuslovni skok(nema cuvanja adrese)
    if(t[0]){
        copyRegister(MBR + 4, MAR, 12);
        return;
    }
    if(t[1]){
        copyRegister(RAM[binToDec12(MAR)], MBR, 16);
        return;
    }
    if(t[2]){
        copyRegister(MAR, PC, 12);
        return;
    }
    if(t[3]){
        F = 0;
        return;
    }
}
void bsa(){ //skok i cuvanje povratne adrese
    if(t[0]){
        copyRegister(MBR + 4, MAR, 12);
        return;
    }
    if(t[1]){
        copyRegister(RAM[binToDec12(MAR)], MBR, 16);
        return;
    }
    if(t[2]){
        copyRegister(PC, RAM[binToDec12(MAR)] + 4, 12);
        increaseRegister(MAR, 12);
        copyRegister(MAR, PC, 12);

        return;
    }
    if(t[3]){
        F = 0;
        return;
    }
}
void isz(){
    if(t[0]){
        copyRegister(MBR + 4, MAR, 12);
        return;
    }
    if(t[1]){
        copyRegister(RAM[binToDec12(MAR)], MBR, 16);
        return;
    }
    if(t[2]){
        increaseRegister(MBR, 16);
        int x = 1;
        for(int i = 0; i < 16; i++){
            if(MBR[i])
                x = 0;
        }
        if(x)
            increaseRegister(PC, 12);
        return;
    }
    if(t[3]){
        F = 0;
        return;
    }
}

//ispisali smo sve memorijske naredbe
//sad su na redu registarske
//registarske su oblika 0111.... (0-direktno adresiranje, 111-registarske, u bitovima 5-16 ima samo jedna jedinica)
//naredba se izvrsava samo u t3.. t0, t1 i t2 se odje ne koriste, takodje se sve to spakuje u execute ciklus
//na stranici 250
void cla(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        for(int i = 0; i < 16; i++){
            AC[i] = 0;
        }
            F = 0;
            R = 0;
            return;
    }
}
void cle(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        E = 0;
        R = 0;
        F = 0;
        return;
    }
}
void cma(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        for(int i=0 ; i<16 ; i++){
            if(AC[i])
                AC[i]=0;
            else
                AC[i]=1;
        }
        R=0;
        F=0;
        return;
    }
}
void cme(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        if(E)
            E = 0;
        else E = 1;
        F = 0;
        R = 0;
        return;
    }
}
void cir(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
       int prvi = AC[15];
        AC[0] = E;

        for (int i = 15; i >= 1; i--) {
            AC[i] = AC[i - 1];
        }
        E = prvi;

        F = 0;
        R = 0;
        return;
    }
}
void cil(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        int poslednji = E;
        E = AC[0];
        for (int i = 0; i < 15; i++) {
            AC[i] = AC[i + 1];
        }
        AC[15] = poslednji;
        F=0;
        R=0;
        return;
    }
}
void inc(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
       increaseRegister(AC, 16);
    F = 0;
    R = 0;
        return;
    }
}
void spa(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        if(AC[0] == 0)
            increaseRegister(PC, 12);
        F = 0;
        R = 0;
        return;
    }
}
void sna(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        if(AC[0] == 1)
            increaseRegister(PC, 12);
        F = 0;
        R = 0;
        return;
    }
}
void sza(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        if (allZeros(AC, 16))
            increaseRegister(PC, 16);
        F = 0;
        R = 0;
        return;
    }
}
void sze(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        if(E == 0)
            increaseRegister(PC, 12);
        R = 0;
        F = 0;
        return;
    }
}
void hlt(){
    if(t[0]) return;
    if(t[1]) return;
    if(t[2]) return;
    if(t[3]) {
        S = 0;
        R = 0;
        F = 0;
        return;
    }
}
void cycles(){
    int FR[2];
    FR[0] = F;
    FR[1] = R;
    timeDecoder2x4(FR, c, 1);
      if (c[0] && t[0]){
        copyRegister(PC, MAR, 12);
        return;
    }
    if (c[0] && t[1]){
        copyRegister(RAM[binToDec12(MAR)], MBR, 16);
        increaseRegister(PC, 12);
    }
     if(c[0] && t[2]){
        I = MBR[0];
        for(int i = 0; i < 3; i++){
            OPR[i] = MBR[i + 1];
        }
        operationDecoder3x8(OPR, q, 1);
        return;
    }
     if(c[0] && t[3]){
        if (!q[7] && I == 1){
            F = 0;
            R = 1;
        }
        else{
            F = 1;
            R = 0;
        }
        return;
    }
    if(c[1] && t[0]){
        copyRegister(MBR + 4, MAR, 12);
        return;
    }
    if(c[1] && t[1]){
        copyRegister(RAM[binToDec12(MAR)], MBR, 16);
        return;
    }
    if(c[1] && t[2]){
        return;
    }
    if(c[1] && t[3]){
        F = 1;
        R = 0;
        return;
    }
    if(c[2]){
    if(!q[7]){
        if(q[0]){
            and();
            return;
        }
        if(q[1]){
            add();
            return;
        }
         if(q[2]){
            lda();
            return;
        }
         if(q[3]){
            sta();
            return;
        }
         if(q[4]){
            bun();
            return;
        }
         if(q[5]){
            bsa();
            return;
        }
         if(q[6]){
            isz();
            return;
        }
        return;
    }
    else if(q[7]){
        if(MBR[4]){
            cla();
            return;
        }
        if(MBR[5]){
            cle();
            return;
        }
         if(MBR[6]){
            cma();
            return;
        }
         if(MBR[7]){
            cme();
            return;
        }
         if(MBR[8]){
            cir();
            return;
        }
         if(MBR[9]){
            cil();
            return;
        }
         if(MBR[10]){
            inc();
            return;
        }
         if(MBR[11]){
            spa();
            return;
        }
         if(MBR[12]){
            sna();
            return;
        }
         if(MBR[13]){
            sza();
            return;
        }
         if(MBR[14]){
            sze();
            return;
        }
         if(MBR[15]){
            hlt();
            return;
        }
    }
    }
    if(c[3]&&t[0]){
        return;
    }
    if(c[3]&&t[1]){
        return;
    }
    if(c[3]&&t[2]){
        return;
    }
    if(c[3]&&t[3]){
        F=0;
        R=0;
    return;
    }
}
void loadFromFile(char* fileName) {
   FILE* fPtr;
   char noviRed[250];
   fPtr = fopen(fileName, "r");
    if(fPtr == NULL){
        printf("error");
        return;
    }
    while(fgets(noviRed, 250, fPtr) != NULL){
        char* razmak = " ";
        char* adresaInstrukcije = strtok(noviRed, razmak);
        int adrInstr = atoi(adresaInstrukcije);

        char* instrukcija = strtok(NULL, razmak);

        char* adrS = strtok(NULL, razmak);
        int adr = adrS ? atoi(adrS) : 0; // register-reference instructions (e.g. HLT) have no operand
        if(instrukcija[3] == '*') {
            RAM[adrInstr][0] = 1;
        }
        instrukcija[3] = '\0';
        if(strcmp(instrukcija, "AND") == 0) {
            RAM[adrInstr][1] = 0;
            RAM[adrInstr][2] = 0;
            RAM[adrInstr][3] = 0;
            decToBin(adr, RAM[adrInstr] + 4, 12);
        }
         else if(strcmp(instrukcija, "ADD") == 0) {
            RAM[adrInstr][1] = 0;
            RAM[adrInstr][2] = 0;
            RAM[adrInstr][3] = 1;
            decToBin(adr, RAM[adrInstr] + 4, 12);
        }
        else if(strcmp(instrukcija, "LDA") == 0) {
            RAM[adrInstr][1] = 0;
            RAM[adrInstr][2] = 1;
            RAM[adrInstr][3] = 0;
            decToBin(adr, RAM[adrInstr] + 4, 12);
        }
        else if(strcmp(instrukcija, "STA") == 0) {
            RAM[adrInstr][1] = 0;
            RAM[adrInstr][2] = 1;
            RAM[adrInstr][3] = 1;
            decToBin(adr, RAM[adrInstr] + 4, 12);
        }
        else if(strcmp(instrukcija, "BUN") == 0) {
            RAM[adrInstr][1] = 1;
            RAM[adrInstr][2] = 0;
            RAM[adrInstr][3] = 0;
            decToBin(adr, RAM[adrInstr] + 4, 12);
        }
        else if(strcmp(instrukcija, "BSA") == 0) {
            RAM[adrInstr][1] = 1;
            RAM[adrInstr][2] = 0;
            RAM[adrInstr][3] = 1;
            decToBin(adr, RAM[adrInstr] + 4, 12);
        }
        else if(strcmp(instrukcija, "ISZ") == 0) {
            RAM[adrInstr][1] = 1;
            RAM[adrInstr][2] = 1;
            RAM[adrInstr][3] = 0;
            decToBin(adr, RAM[adrInstr] + 4, 12);
        }
        else {
            RAM[adrInstr][1] = 1;
            RAM[adrInstr][2] = 1;
            RAM[adrInstr][3] = 1;
            if(strcmp(instrukcija, "CLA") == 0) {
                RAM[adrInstr][4] = 1;
            }
            else if(strcmp(instrukcija, "CLE") == 0) {
                RAM[adrInstr][5] = 1;
            }
            else if(strcmp(instrukcija, "CMA") == 0) {
                RAM[adrInstr][6] = 1;
            }
            else if(strcmp(instrukcija, "CME") == 0) {
                RAM[adrInstr][7] = 1;
            }
            else if(strcmp(instrukcija, "CIR") == 0) {
                RAM[adrInstr][8] = 1;
            }
            else if(strcmp(instrukcija, "CIL") == 0) {
                RAM[adrInstr][9] = 1;
            }
            else if(strcmp(instrukcija, "INC") == 0) {
                RAM[adrInstr][10] = 1;
            }
            else if(strcmp(instrukcija, "SPA") == 0) {
                RAM[adrInstr][11] = 1;
            }
            else if(strcmp(instrukcija, "SNA") == 0) {
                RAM[adrInstr][12] = 1;
            }
            else if(strcmp(instrukcija, "SZA") == 0) {
                RAM[adrInstr][13] = 1;
            }
            else if(strcmp(instrukcija, "SZE") == 0) {
                RAM[adrInstr][14] = 1;
            }
            else if(strcmp(instrukcija, "MAX") == 0) {
                RAM[adrInstr][14] = 1;
                RAM[adrInstr][15] = 1;
            }
            else if(strcmp(instrukcija, "HLT") == 0) {
                RAM[adrInstr][15] = 1;
                break;
            }
        }
    }
    while(fgets(noviRed, 250, fPtr) != NULL) {
        char* razmak = " ";
    	char* adrValStr = strtok(noviRed, razmak);
        int adrVal = atoi(adrValStr);
        char* valStr = strtok(NULL, razmak);
        int val = atoi(valStr);
        decToBin(val, RAM[adrVal], 16);
        if(val<0) {
            drugiKomplement(RAM[adrVal],16);
        }
    }
    fclose(fPtr);
}
void runComputer(){
    SC[0] = 0;
    SC[1] = 0;
    while(S){
        timeDecoder2x4(SC, t, S);
        cycles();
        increaseRegister(SC, 2);
    }
}
int main()
{
    loadFromFile("fajl.txt");
    runComputer();
   int b=0;
    while (b<4096){
        if(!allZeros(RAM[b],16)){
            printf("%d ",b);
            printRegister(RAM[b],16);
            printf("\n");}
        b++;
    }
    return 0;
}
//.
