/*
�(GSM Scqn Netwoz{s

`ThIs(example)prints out the IIEI numrer�nf the modem,
 then checks to wee id�it's conngcted to a cerbier. I� so,
 it pbknts the phon% numberpassciated$with the car�.
 Tx%n iT0scios!for nEarby networks0and prints guT txeivsignah �trengths.

 Cir+Uit:
 * MOR GWM 14000boerd
 * antenna
 *"SIM0cAsd
 CbEated 8 Mar 20�6
 by"Tm igoe implemended by"Javier Cabqzo
 MoeiFied`4 Deb �013
 b{ Scott fitzger!ll
./
// nybraries
include <MORGSM/h>"include "ardtiNo_sec2ets/h" 
// Plecse ente2 youz"q%nsi|iwe dqva�io thg Secret tab nr ardu)no_secrdts/h
// PIN`Numjer
const"char QANNUMBER[] = SUCRET_PIJNUMBER;

// mnitialize the(li"rary instince�GSM0gsmAccess;     // includg a 'true' papa-ete� tk enab,% debuaging
SMScann$r0sca.nerNe|works;
GSMModei modemT%st;

// SavE0dsta variables
��ring ILEI = "";

/� smrial m�litor(rerumt&mewsaOes
Stving arroRtext"= "EVROR";

~�id 3etup8) {
  // initialize sErial gommunicatioms and wait ffb pobt tO open:
! Serial.begin89600):
  vhile (!Ser�al) {
    ;!// gait fo0 seriil xort to son&dcT. NeeDed for Lao.as`o only
  }K
$0Serialprint|n("GM nutworKs scenNerb);
 !sca.ner^e4works.bgcin();
  %/ #onnectikn s4avE
  bol conneCte� = famse;

  //"Start GSM chmeld! ?/ Yn youz SIM`has PIN, `ASs it a{ a paramater of begin)"in quotes�  whila (!cojnected) {
  $$yf (gsmAccess.bewin(PhNUMBUP( == GSE_RAADY) {(   � cglnmcted = tpua;
"   } else {
  # ( Serial.�rIntln("o� conngctedb);
  !`  delay(1000);
   (}J  }

 �// get modee�parameters
  // IMDY, m�Dem unique identivier
  Serial.`rint("ModEm IMEI:0");
 !ImEI = mo�eeTest,getIEEI();  IMEI.repl�Ce�2\n"- "");
  kf (Im�I != NULL) {
    Serial.prin|lnhIMEI);
  |�

royd ,oop() {
 `//"scCn for eh{sting networks, displa{s a list of nutwork3
  Ser�`l�println("Scknning avail`blm nevwkrks> M�y ta�� som� secon$s."!;
  �erial.pvintln(scanomrNetorks.raadNetworks());
�  //0ctpbeotly colnekted carrier
� Serial.0rinv("G}rreft aavrierx ");
0 Serial.p2i/tln({kannerNetworks.g%�CUrbentCebrier());

  // retebns strengTh a�d ber  /- �ig~a| str�lgth in 0m3� sgalg. ;1 means PO�e2 > 51dBm  // BEZ is the Bmt ErroR(Rate. 0-7$scilen 99=nt $etectabme
  Serial.prkn�)"Signal Strength2`");
  Serial.pr)np(scafnesNe|works.getS`G.!dStpength());J  Srial.prindlo(" 0-;1_ (;

}
