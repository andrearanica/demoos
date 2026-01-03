# Memoria virtuale

## Funzionamento
- Dimensione pagine: 4 Kb
- 4 livelli di gerarchia di pagine

- In demoos useremo solamente 16 pagine per i processi
- L'indice di pagina all'interno dell'indirizzo virtuale è formato dai bit 47-39
- Serve solo una tabella che punterà direttamente all'indirizzo fisico
  - PGD --> PUD --> PMD --> PTE (le prime 3 avranno una sola entry verso la tabella successiva)
- Ogni processo ha un proprio set di queste 4 tabelle, e quindi quando faccio il context switch devo caricare la tabella del processo che viene lanciato
  - La PGD del processo in esecuzione va caricata nel registro `ttbr0_el1`
  - N.B. Nel nostro caso ci servono solo 4 tabelle delle pagine per ogni processo; se avessimo bisogno di più pagine potrebbero servire più di una PUD/PMD/PTE

- La conversione da indirizzo virtuale a fisico viene fatta dalla MMU
  - Leggo i bit 47-39 per trovare la riga nella PGD, poi altri bit per le altre tabelle e i bit 11-0 sono quelli che indicano l'offset all'interno della pagina
- Importante: nel nostro caso ci basterà riempire ogni riga della PGD dello stesso indirizzo della PUD, ogni riga della PUD dello stesso indirizzo della PMD e ogni riga della PMD dello stesso indirizzo della PTE ==> ci serve solametne l'ultimo livello dato che abbiamo solo 16 pagine

- Ogni entry della tabella delle pagine si chiama descrittore. Contiene l'indirizzo della tabella successiva, ma siccome l'indirizzo è allineato alla pagina i bit 11-0 sarebbero a zero; invece che lasciarli a zero scrive le seguenti informazioni:
  - 0: bit di validità; se durante una traduzione è = 0, viene generata un'eccezione e il kernel deve allocare una pagina e modificare il descrittore
  - 1: se il descrittore punta alla tabella successiva della gerarchia, o se punta direttamente a un indirizzo fisico
    - Domanda: sfruttiamo questa cosa in modo da usare solamente una tabella, oppure seguiamo la gerarchia vera?
  - 63-48: altri dati
- Alcuni attributi non sono scritti direttamente nel descrittore, ma si trovano nel registro `mair_el1`; questo registro ha 8 sezioni da 8 bit, e il descrittore contiene un offset di 3 bit (2^3=8 sezioni) che punta alla sezione di questo registro con le informazioni utili per la pagina scelta
  - In demoos, useremo solo due sezioni: la prima per la memoria normale, la seconda per la porzione di memoria che non va caricata in cache (come quella IO)

- Servono almeno due tabelle delle pagine diverse: una per il processo in esecuzione e una per il kernel. Visto che non vogliamo ricaricare il registro che contiene l'indirizzo della PGD ad ogni cambio di EL, abbiamo due registri `ttbr0_el1` e `ttbr1_el1`:
  - Se un'indirizzo virtuale inizia con `0x0000`, allora verrà usata la tabella `ttbr0_el1`
  - Se inizia con `0xffff`, allora verrà usata la tabella `ttbr1_el1`
- In questo modo lo spazio degli indirizzi è diviso in due: gli indirizzi 0xffff virtuali possono essere acceduti solo dal kernel, mentre il resto è per l'utente. 

## Kernel
- Inizializziamo le tabelle per il kernel: definiremo una procedura assembly `__create_page_tables` che farà i seguenti passi
  - Abbiamo bisogno di mappare 1 GB di memoria (perché è quella che ha il raspberry)
  - Ci serve una tabella PGD che punta a una sola tabella PUD che punta a una sola tabella PMD: visto che la PMD può puntare direttamente a blocchi da 2MB e ha 512 entry, permette di mappare tutto il 1 GB (possiamo evitare la PTE)
  - Visto che ogni tabella delle pagine è grande 1 pagina, dobbiamo allocare 3 pagine di memoria per mantenere queste 3 tabelle (cioè imposto a zero lo spazio di memoria di 3 pagine)

- Definiamo la macro `create_table_entry` che servirà per scrivere una entry nella tabella delle pagine (nel kernel verrà chiamata per la creazione di PGD e PUD, mentre per la PMD useremo un'altra procedura)
  - I parametri che accetta sono:
    - `tbl`: indirizzo di memoria in cui allocare la tabella
    - `virt`: indirizzo virtuale di cui vogliamo scrivere la entry
    - `shift`: numero di bit da shiftare per ottenere l'indice nella tabella (39 per la PGD e 30 per la PUD)
    - `tmp1, tmp2`: due registri temporanei
  - Cosa fa questa macro?
    - Tramite shift e and calcola l'indice della nuova entry nella tabella, partendo dall'indirizzo virtuale
    - Calcola l'indirizzo della tabella successiva: visto che abbiamo 3 tabelle delle pagine, le memorizziamo in 3 pagine adiacenti e quindi l'indirizzo sarà quello della tabella + 1 pagina
    - Devo prende quell'indirizzo e mettere gli ultimi due bit a 1: bit 1 dice che punta a una tabella e non a un'indirizzo fisico, bit 0 è il bit di validità
    - Scrivo nell'indice della tabella calcolato prima il descrittore appena calcolato
    - Per comodità, incrementiamo `tbl` della dimensione di una pagina in modo che punti già alla tabella successiva

- La macro `create_block_map` serve invece per scrivere una entry nella PMD
  - Occhio: a differenza della macro di prima, questa può essere usata per scrivere più entry di fila (possiamo mappare più settori di 2MB)
  - I parametri che servono sono:
    - `tbl`: indirizzo della PMD
    - `phys`: indirizzo fisico su cui vogliamo mappare il settore
    - `start` e `end`: il primo e l'ultimo indirizzo virtuale che vogliamo mappare
    - `flags`: vengono sommati a `phys` per venire scritti nella entry della tabella
  - Cosa fa questa macro?
    - Estrae l'indice nella tabella della entry per start e per end, come faceva quella di prima, così abbiamo il primo e l'ultimo indice della tabella
    - Per calcolare il descrittore, prendo `phys` e lo shifto a destra e sinistra di 21 bit per cancellare i primi 21 bit da destra (dato che l'offset mi viene dato dall'indirizzo virtuale)
    - Faccio l'or tra quello calcolato prima e le flag, e ho ottenuto il descrittore
    - In loop fino a quando start < end, scrivo il descrittore e incremento l'indice corrente

- Creo una macro che al suo interno va a chiamare due volte la `create_table_entry` passando i registri:
  - x0: in cui salvo l'indirizzo della PGD
  - x1: primo indirizzo virtuale da mappare (0xffff000000000000)
  - x2 e x3 come registri temporanei
- Dopo l'esecuzione di questa macro, x0 sarà stato incrementato e punterà alla PMD

- All'interno di boot.S dobbiamo quindi mappare tutta la memoria per essere usata dal kernel, chiamando la `create_block_map` su tutto lo spazio (esclusi i registri MMIO, che vanno mappati successivamente con diversi flag)

- Per finire devo fare un altro po' di operazioni:
  - Imposto lo stack pointer all'indirizzo virtuale corretto invece che quello fisico
  - Salvo in `ttbr_el1` la PGB creata prima
  - Salvo in `tcr_el1` dei parametri di configurazione per la traduzione degli indirizzi, tipo la dimensione delle pagine kernel e user
    - Il valore da scrivere nel registro è preparato in un file `.h`
  - Imposto anche il valore del registro `mair` (stessa cosa di prima, il valore è fisso)
  - Infine, salvo in `x2` l'indirizzo del kernel_main, abilito la MMU e salto al kernel main
    - Devo prima memorizzare la kernel_main perché con la MMU attiva gli indirizzi sarebbero errati

## User space

- Ogni processo utente ora deve avere il proprio spazio di indirizzamento
- Il codice binario del programma utente va caricato dal file system

- Va modificata la funzione `move_to_user_space`
  - Avrà bisogno di 3 argomenti:
    - Nome del file contenente il binario da eseguire
    - Offset per la funzione main al suo interno (cioè il nuovo PC virtuale)
  - Viene impostato lo stato del registro e il program counter virtuale
  - Lo stack pointer dovrà puntare all'ultimo indirizzo della pagina 1
  - Il codice del programma viene preso dal filesystem e copiato dentro alla 16a pagina del processo
    - Viene chiamata la `allocate_user_page` per allocare una pagina in cui scrivere il codice; l'indirizzo virtuale sarà il primo della 16a pagina
  - Viene anche impostata la pgd del processo corrente

- Come funziona la `allocate_user_page`?
  - Ottiene una pagina di memoria libera
  - Mappa l'indirizzo virtuale fornito su quella pagina tramite la `map_page`
  - Ritorna l'indirizzo fisico 0xffff + numero di pagina, che è l'indirizzo virtuale del kernel in cui verrà copiato il sorgente del programma

- La `allocate_user_page` chiama la `map_page` per mappare la pagina appena allocata nelle tabelle delle pagine del processo utente
  - Se il processo non ha una pgd, la crea e la salva nell'array `kernel_pages` per ricordarsela
  - Chiamo la funzione `map_table` per 3 volte, per andare a scrivere nelle tabelle PGD, PUD e PMD le entry necessarie
    - Se durante la creazione delle entry la tabella non esiste, la creo al momento
  - Salva una struct indirizzo fisico pagina - indirizzo virtuale pagina nell'array user_pages del processo

- La funzione `map_table` è quella responsabile di scrivere un descrittore che punta alla tabella successiva
  - Calcola l'indice che l'indirizzo virtuale fornito avrà nella tabella
  - Se la riga è vuota, vuol dire che non ho ancora la tabella successiva nella gerarchia
    - Chiedo una pagina libera, faccio l'or con la flag e scrivo in `table[index]` quel valore
  - Altrimenti ritorno il valore scritto in quella entry

- La funzione `map_table_entry` fa la stessa cosa della `map_table`, solo che scrive nell'indice calcolato l'indirizzo fisico della pagina

- Quindi alla fine di tutto, ogni processo ha due attributi importanti:
  - Kernel pages: mantiene l'elenco di tutte le pagine allocate a un processo
  - User pages: mantiene la coppia pagina fisica - pagina virtuale per ogni pagina associata al processo

### Fork

- Dobbiamo fare in modo che la fork crei una copia dello spazio degli indirizzi del processo padre, e che ritorni il PID se sei nel padre e 0 se sei nel figlio
- Ora la copy_process dovrà copiare l'intero spazio degli indirizzi virtuale del processo padre, tramite la `copy_virt_memory`
- La `copy_virt_memory` dovrà:
  - Per ogni pagina associata al processo utente corrente, chiedo una nuova pagina utente con lo stesso indirizzo virtuale e ci copio il contenuto della pagina utente
