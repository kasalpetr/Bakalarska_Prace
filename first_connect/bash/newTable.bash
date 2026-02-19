#vytvoreni nove tabule na miro
curl --request POST      --url https://api.miro.com/v2/boards      --header 'accept: application/json'      
--header 'authorization: Bearer eyJtaXJvLm9yaWdpbiI6ImV1MDEifQ_JIhCDDG4eg1ch_5_XsYIOoAp-_E'      
--header 'content-type: application/json'      --data '{"name":"Moje prvni tabule přes API"}'
