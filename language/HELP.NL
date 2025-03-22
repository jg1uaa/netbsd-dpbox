HELP-INFO voor DigiPoint-Box (14.03.1993):
-----------------------------------------

Commando's zijn:

CHECK, DIR, ERASE, HELP, LIST, MYBBS, NAME, QUIT, READ, SEND,
TRANSFER, USAGE, LOGINTIME, TIME, BBS, SPEAK, LANGUAGE, REPLY, COMMENT

CHECK : List de nieuwe berichten naar datum gesorteerd, bv. 'C 10'
        geeft de laatste 10 nieuwe berichten.
        Alleen 'C' geeft alle nieuwe berichten sinds laatste inlog. 
        'C < test' geeft alle berichten die in de titel 'Test'
        hebben staan.
        
BBS   : Vraagt verder informaties over een Mailbox. Syntax:
        BBS <CALL> . Geeft men alleen prefix in, dan verschijnt
        een lijst met alle mailboxen die deze BBS kent.
        Met 'BBS < Hamburg' kan men extra infornmatie zoeken
        zover deze bekent is.
        Mailboxen die hier niet in voorkomen kunt u natuurlijk wel
        berichten sturen.

DIR   : Geeft een lijst met alle areas in deze BBS.

ERASE : Wist alle berichten die u zelf heeft geschreven of aan u gericht 
        zijn, bv. 'E PA3ZZZ 3-5' 

HELP  : Deze help file.

LIST  : List alle berichten in een area met aanduiding van Datum,
        Afzender en Lengte. Ook hier kan men het bereik aangeven:
        'L PA3ZZZ 3-5' of 'L ALLE 1-5' enz.

MYBBS : Invoeren van uw HomeBBS, hier niet u eigen call ingeven!! 
        (ook niet als u zelf een BBS heeft) 
        bv. MYBBS PI8AIR

NAME  : Invoeren van uw naam, dan word u bij een volgende connect
        persoonlijk begroet,  bv. 'N ERWIN'.

QUIT  : De BBS zal de log-in wegsaven en dan de verbinding verbreken.
        Normaal disc. is ook mogelijk, maar dan word de log-in niet
        geupdate.

READ  : Lezen van berichten, een bereik aangeven kan ook.
        bv. 'R PA3ZZZ 2-5' oder 'R ALLE 1-5'.

SEND  :  Versturen van een bericht in de Mailbox,
         bv. 'S DL8HBS' of 'S DL8HBS (Titel)'. 
         Het bericht afsluiten met een 'CTRL Z' of '***END'.
         Met 'S DL8HBS #10 (Titel)' geeft men het bericht een
         levensduur (lifetime) van 10 dagen, daarna word ze automatisch
         gewist. Waneer een bericht aan een user naar een andere Mailbox
         verstuurd moet worden let er dan op dat ook de bestemmings-BBS
         aangeven moet worden. bv. 'S DL8HBS @ DB0GR (Titel)' 

SPEAK : Hiermee kunt u de Taal-keuze veranderen. 
        bv. SPE NL
        LANguages laat alle aanwezige Talen zien waaruit u kunt kiezen. 
        NLX, DLX etc. is de zgn. eXpert-mode (verkorte texten).

TRANSFER: Verplaatsen van eigen berichten. Persoonlijke berichten
          kunnen vermeervoudigt worden, Bulletin's in een bepaalde area
          kunnen zo gemakkelijk in een andere area geplaatst worden.
          bv. 'T PA3ZZZ 3-5 > PE1XXX'

USAGE : Lijst van geconnecte users op de BBS. 'U PA3ZZZ' geeft info van  
        PA3ZZZ zover die info bekent is.

LOGINTIME: Geeft de datum van de laatst gemaakte verbinding met deze BBS. 
           Met 'LOG 17.04' of 'LOG 17.04.92' kunt u deze datum veranderen,
           dit is vooral handig bij het gebruik van Check-Commando.

TIME   : Geeft de aktuele tijd.

REPLY  : Hiermee kunt u direkt het laatst gelezen bericht beantwoorden.
         Als titel krijgt men dan RE:<orginele titel> en word het verstuurt
         naar de afzender van dat bericht.
         Met REPLY <nieuwe titel> krijgt men een nieuwe titel.
        
COMMENT: Deze funktie is hetzelfde als REPLY, echter stuurt deze een 
         bericht naar de laatstgelezen area.
         Gaat het om Peroonslijke mail (User-area), dan reageert deze 
         funktie hetzelfde als REPLY. 


73's 

