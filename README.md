# myfind
light version of the UNIX command "find", without using the getopt(3) funktion

Anleitung:

Üblicherweise beginnen Programme mit einer vollständigen Analyse (z.B. mit getopt(3)) und Überprüfung der Parameter. Bei find(1) dürfen Sie getopt(3) nicht verwenden, weil damit unzulässige Parameterangaben wie --name fälschlicherweise erlaubt wären.

Weiters reicht es für dieses Bsp. aus, wenn Sie die Parameterüberprüfung direkt während der eigentlichen Auswertung machen. Dies wird nicht die "technisch beste" Implementierung ergeben, aber dafür den Aufwand in Grenzen halten und eine relativ übersichtliche Lösung ergeben.

Ebenso muß myfind einige Fehler bei System-Calls "überleben", d.h. daß ein einfaches exit(3) in vielen Fällen keine korrekte Fehlerbehandlung sein kann.

Bauen Sie ihr Programm um 2 Funktionen auf:

do_file(const char * file_name, const char * const * parms)
wird für jedes zu testende File aufgerufen. parms ist dabei die Adresse des ersten Arguments (so wie beim argv), das eine Aktion ist. do_file() ruft dabei für jedes Subdirectory do_dir() geeignet auf.
do_dir(const char * dir_name, const char * const * parms)
wird für jedes zu testende Directory aufgerufen. parms ist dabei die Adresse des ersten Arguments (so wie beim argv), das eine Aktion ist. do_dir() liest das Directory aus (opendir(3), readdir(3) und closedir(3)) und ruft dabei für jeden Directory-Eintrag do_file() geeignet auf.
<aktion> kann sein
-user	<name>/<uid>	finde Directoryeinträge eines Users
-name	<pattern>	finde Directoryeinträge mit passendem Namen
-type	[bcdpfls]	finde Directoryeinträge mit passendem Typ
-print		gibt den Namen des Directoryeintrags aus
-ls		gibt die Nummer des Inodes, Anzahl der Blocks, Permissions, Anzahl der Links, Owner, Group, Last Modification Time und den Namen des Directoryeintrags aus. Bei Sym-Links wird auch das Ziel ausgegeben werden. Die Ausgabe soll in etwa wie die des bereits vorhandenen find(1) aussehen, z.B.:

    {3}find .bashrc -ls 
    8159772    8 -rw-r--r--   1 bernd    bernd        6657 May 27 00:23 .bashrc
              
Benützen Sie dazu readlink(2). Datumsausgaben sind mit strftime(3) so zu formatieren, wie es bei einem (einigermaßen) aktuellen File aussieht. Gehen Sie davon aus, daß nur "normale" Filenamen vorhanden sind — Sie brauchen nicht die speziellen Quoting-Regeln aus dem Abschnitt "UNUSUAL FILENAMES" der Manual-Page implementieren.
Wenn Sie eine 3er Gruppe sind, dann implementieren Sie noch folgende <aktion>en :
-nouser		finde Directoryeinträge ohne User
-path	<pattern>	finde Directoryeinträge mit passendem Pfad (inkl. Namen)
Wenn Sie eine 4er Gruppe sind, dann implementieren Sie noch folgende <aktion>en :
-group	<name>/<gid>	finde Directoryeinträge einer bestimmten Gruppe
-nogroup		finde Directoryeinträge ohne Gruppe
Verwenden Sie Unterfunktionen, um Unteraufgaben zu implementieren, und vermeiden sie doppelten/kopierten Code.

Die Aktionen werden von links nach rechts überprüft und ausgeführt. Sobald eine Aktion nicht erfüllt ist, kann die Bearbeitung des aktuellen Filenamens abgebrochen werden (sollte es sich um eine Directory handeln, muß myfind natürlich trotzdem im Directory weitermachen - so wie beim "echten" find(1)). Die Aktionen werden sozusagen logisch und verknüpft und können Seiteneffekte wie eine Ausgabe haben (so wie beim "echten" find(1)).

Den Inhalt eines Inodes - die Metadaten eines Files - bekommen Sie mit den SysCalls stat(2) bzw. lstat(2).

Die Option -user soll einen Usernamen oder eine numerische User-Id als Argument verarbeiten. Im Inode des Files wird lediglich die numerische User-Id gespeichert. Verwenden Sie die Library-Funktion getpwnam(3), um nach dem Eintrag des Users in /etc/passwd zu suchen.

Die Option -nouser testet, ob ein User mit der User-Id des Files existiert. Verwenden Sie die Library-Funktion getpwuid(3), um in /etc/passwd zu suchen.

Die Optionen -name und -path sollen Filenamen-Pattern als Argument verarbeiten. Verwenden Sie die Library-Funktione fnmatch(3), um diese Funktionalität zu erreichen.

Die Option -group soll einen Gruppennamen oder eine numerische Gruppen-Id als Argument verarbeiten. Im Inode des Files wird lediglich die numerische Gruppen-Id gespeichert. Verwenden Sie die Library-Funktion getgrnam(3), um nach dem Eintrag des Users in /etc/group zu suchen.

Die Option -nogroup testet, ob eine Gruppe mit der Gruppen-Id des Files existiert. Verwenden Sie die Library-Funktion getgrgid(3), um in /etc/group zu suchen.

Sollte etwas unklar oder verwirrend sein, ist das Verhalten des "echten" find(1) in diesem Fall zwingend als Vorgabe zu verwenden. D.h. das sie u.U. auch das zu implementierende Verhalten durch "ausprobieren" ermitteln können und/oder müssen.
