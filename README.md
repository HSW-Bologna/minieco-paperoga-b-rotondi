# Minieco Paperoga B

## TODO

 - Chiedere gestione umidita': se va oltre un certo valore (parametro) ferma il tempo di asciugatura (ma continua ad asciugare). Riparti se scende sotto il limite per piu' di 3 minuti (solo in modalita' automatica). Il tutto solo se e' collegata la sonda di temperatura/umidita'.

 - Ventilazione (FATTO): se la macchina e' ferma e
                    - il tempo ventilazione oblo' aperto e' zero ferma la ventilazione
                    - l'oblo' e' chiuso ferma la ventilazione
                    - l'oblo' e' aperto fai girare la ventilazione per il tempo (parametro)
                        - se viene chiuso o il tempo finisce fermala
                - se la macchina e' al lavoro e
                    - Sei in asciugatura o raffreddamento vai
                    - in antipiega vai solo mentre il cesto gira

 - Presenza aria (FATTO): se sei fermo o la ventilazione e' ferma non fare nulla. Se sto andando e non sento flusso aria (vedi ingresso) per il tempo configurato (parametro) vado in allarme (auto rientrante?)

 - Blocco bruciatore (FATTO): se sento il segnale di blocco bruciatore per due secondi attivo il reset del blocco bruciatore per 3 secondi e poi ne aspetto 5 prima di ricominciare. Se supero il numero di tentativi specificati (parametro) attivo l'allarme.

 - A cosa serve l'inversione velocita'? e l'abilitazione attesa temperatura?
