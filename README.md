## Router
### MOE-ISC-DHCP-SERVER
La guida si riferisce alla versione 4.2.4 di debian-isc-dhcp.
Scaricare il codice sorgente con il comando:
	git clone https://github.com/marschap/debian-isc-dhcp.git

Creare un file vuoto che conterrà il log dei leases tramite il comando:
	sudo touch /var/lib/dhcp/dhcpd.leases

Creare il file
	/etc/dhcp/dhcpd.conf

con la configurazione del server dhcp (esempi di configurazione possono essere trovati nel file debian-isc-dhcp/server/dhcpd.conf).

Sostituire il file "db.c" presente nella directory debian-isc-dhcp/server/ con quello fornito.
Eseguire i seguenti comandi:
	./configure
	make
	sudo make install

Spostare lo script moe-isc-dhcp-server in /etc/init.d/
Lanciare il seguente comando per avviare il server al boot del sistema
	sudo update-rc.d moe-isc-dhcp-server defaults

Modificare le seguenti righe dello script moe-isc-dhcp-server
	export MOE_CONNECTOR_PATH="path di moe-sender(vedi Router->MOE-SENDER)"
	export RECEIVER_ADDRESS="ip xxx.xxx.xxx.xxx della macchina che ospita moe-receiver" # vedi Server->MOE-SERVER)
	export RECEIVER_PORT="porta utilizzata per contattare moe-server, di default 5193" 
	
configurazione di default:
	export MOE_CONNECTOR_PATH=/root/moe-sender/moe-sender
	export RECEIVER_ADDRESS=192.168.1.2
	export RECEIVER_PORT=5193 	
		
Configurare il server dhcp editando il file /etc/dhcp/dhcpd.conf come un normale server isc-dhcp-server facendo solamente attenzione ai due parametri:
	default-lease-time 300;
	max-lease-time 300;

Più questi parametri saranno bassi più il margine di errore del vostro sistema presenze sarà minore. Naturalmente esagerare mettendo un lease time troppo basso potrebbe compromettere la vostra rete. Nel nostro sistema abbiamo trovato un giusto compromesso settando il lease time a 300 secondi(5 minuti).


### MOE-SENDER
Compilare moe-sender eseguendo semplicemente il comando make dentro la cartella moe-sender.
Assicurarsi che il path della variabile di ambiente MOE_CONNECTOR nello script moe-isc-dhcp-server e il path del file appena compilato corrispondano, altrimenti spostate uno o editate l'altro.



## Server
### MOE-RECEIVER
Assicurarsi di avere installato il pacchetto per sviluppatori di sqlite3. Per l'installazione del pacchetto su debian e derivate:
	sudo apt-get install -y libsqlite3-dev
			
Se la variabile di ambiente RECEIVER_PORT dello script moe-isc-dhcp-server è stata cambiata dal valore di default(5193) cambiare questa riga del file moe-receiver/header.h:
	#define RECEIVER_PORT   5193

Con il nuovo valore che avete scelto per la porta tcp.
Sempre nel file moe-receiver/header.h cambiate questa riga:
	#define DB_FILE_NAME    "/home/roma2lug/moe-receiver/users.sqlite"
		
mettendo il path della cartella dove andrete a mettere il file compilato moe-receiver e lasicando "/users.sqlite" come ultima parte del path. Compilate eseguendo il comando make all'interno della cartella moe-receiver. Spostate lo script moe-receiverd in /etc/init.d/.
Lanciare il seguente comando per avviare moe-receiver al boot del sistema
	sudo update-rc.d moe-receiverd defaults

Modificare le seguenti righe dello script moe-receiverd
	start-stop-daemon -S --exec /home/roma2lug/moe-receiver/moe-receiver -b

Sostituendo il path /home/roma2lug/moe-receiver/moe-receiver con il path del file compilato moe-receiver.

> NOTA: In una versione futura questi passaggi verranno semplificati con l'inserimento di apposite variabili di ambiente da editare direttamente nello script moe-receiverd, proprio come nella parte Router.
	
### MOE.PHP
Assicurarsi di avere installato uno web-server e di avere installato e abilitato il modulo php. Nel nostro caso ci siamo affidati ad Apache con PHP5.
Spostare il file moe.php nella stessa directory in cui si trova moe-receiver.
Collegate il vostro web-server al file moe.php, se utilizzate Apache vi basterà un link simbolico in /var/www:

```shell
cd /var/www
sudo ln -s "path del file moe.php"
```

### WIDGET.PHP
Come per moe.php spostare o linkare il file al vostro web-server. Nel nostro caso abbiamo utilizzato il codice php del file widget.php come widget in un sito gestito dal CMS Wordpress grazie al [Plugin PHP Code Widget](http://wordpress.org/plugins/php-code-widget/).

Il nostro widget funzionante lo trovate come primo widget nella colonna di destra del nostro sito:

[http://lug.uniroma2.it/](http://lug.uniroma2.it/)

Per dubbi, problemi o chiarimenti non esitate a contattarci:

Sito: http://lug.uniroma2.it/
E-Mail: roma2lug@gmail.com
Facebook: https://www.facebook.com/groups/roma2lug/


_Roma2LUG, Linux User Group dell'Università di Roma Tor Vergata._
