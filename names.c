
/* German Atlantis PB(E)M host Copyright (C) 1996, 1997 Alexander Schroeder
   
   based on:

   Atlantis v1.0 13 September 1993 Copyright 1993 by Russell Wallace

   This program may be freely used, modified and distributed.  It may
   not be sold or used commercially without prior written permission
   from the author.  */

/* This text is included into creation.c  */

char *regionnames[] = 
{
  "Aalschwil",
  "Aberaeron",
  "Aberdaron",
  "Aberdovey",
  "Abernethy",
  "Abersoch",
  "Abrantes",
  "Acanseh",                               
  "Ach'Brkt Ndr'Brkt",
  "Adrano",
  "AeBrey",
  "Aea Bryye K'kcht",
  "Aeey Kcht",
  "Aerg Jaehrn",
  "Aerg Pycht",
  "Aerg Tuun",
  "Agettes",
  "Aghleam",
  "Agr Looch",
  "Aiguilles",
  "Ajkh Brcht",
  "Akbou",
  "Akiri",
  "Aldan",
  "Alfaro",
  "Alghero",
  "Alicahue", 
  "Alleves",
  "Almens",
  "Almeria",
  "Altels",
  "Altnaharra",
  "Amacuya",            
  "Amayuca",            
  "Amecameca",          
  "Amosi",
  "Amozoc",                                
  "Amuzgos",                               
  "Ancroft",
  "Anniviers",
  "Anshun",
  "Anstruther",
  "Antgor",
  "Antifer",
  "Antor",
  "Apan",                                  
  "Aparima",
  "Apizaco",                               
  "Aproz",
  "Arbroath",
  "Arcila",
  "Ardfert",
  "Ardon",
  "Ardvale",
  "Arezzo",
  "Argentan",
  "Ariano",
  "Aris",
  "Arlon",
  "Armina",
  "Arnayes",
  "Arolla",
  "Arven",
  "Asp",
  "Astata",                                
  "Atlacomulco",  
  "Atlatlahuacan",
  "Atlixco",                               
  "Atta Ird",
  "Atzacoaloya",  
  "Atzacualoya",                           
  "Atzompa",            
  "Augstbord",
  "Ault",
  "Auw",
  "Avanos",
  "Aveiro",
  "Aven",
  "Avent",
  "Avouin",
  "Bacalar",            
  "Badalona",
  "Baechahoela",
  "Baie",
  "Ballindine",
  "Balta",
  "Banlar",
  "Barfleur",
  "Barghis",
  "Bargona Etsch",
  "Bargun",
  "Barguns",
  "Barika",
  "Barma",
  "Baselga",
  "Bastak",
  "Batz",
  "Bayonne",
  "Bejaia",
  "Bella-Tolla",
  "Benal",
  "Bendolla",
  "Benlech",
  "Beragh",
  "Beranabadui",
  "Berent",
  "Bergland",
  "Beristain",          
  "Berli",
  "Berneray",
  "Berriedale",
  "Bettwil",
  "Bidemjini",
  "Bihima",
  "Binhai",
  "Birde",
  "Bivenad",
  "Biwalin",
  "Blain",
  "Blakten",
  "Bleiken",
  "Bleis",
  "Bleusy",
  "Bocholt",
  "Bodaner",
  "Bodmen",
  "Bogmadie",
  "Bonder",
  "Borgne",
  "Bornaduz",
  "Botowin",
  "Bovarina",
  "Bovas",
  "Bowisvu",
  "Braga",
  "Brechlin",
  "Bricquebec",
  "Briex",
  "Brignogan",
  "Brkt P'cht",
  "Brodick",
  "Broons",
  "Buhi",
  "Bulag Og",
  "Bulesunun",
  "Bumi",
  "Burscough",
  "Buttino",
  "Butzstafel",
  "Cachapoal", 
  "Calantgil",
  "Calcarida",
  "Calpio",
  "Calvarut",
  "Camadra",
  "Canna",
  "Capperwe",
  "Caprera",
  "Carahue",
  "Carbost",
  "Carcavelos",
  "Carentan",
  "Carhaix",
  "Carnet",
  "Carnforth",
  "Carrigaline",
  "Casatscha",
  "Caschlegia",
  "Caschlera",
  "Caserta",
  "Catemu", 
  "Catrianchi",
  "Cavadroess",
  "Cavarschons",
  "Cazirauns",
  "Chable",             
  "Chable",                                
  "Chacabuco",
  "Chacmultum",                            
  "Champex",
  "Chargeur",
  "Charrat",
  "Chartonnes",
  "Chaulue",
  "Chemax",             
  "Chepec",
  "Cherbourg",
  "Chicxulub",    
  "Chiel Ferd",
  "Chiesset",
  "Chilch",
  "Chincholco", 
  "Chinzg",
  "Chippis",
  "Chirgua",                               
  "Choapa", 
  "Chreien",
  "Chrkt'prch",
  "Chrt vrk't",
  "Chunyaxche",   
  "Chupiquaro", 
  "Clariden",
  "Clatter",
  "Cleguerax",
  "Cleuson",
  "Cleusson",
  "Coatzcoalcas",                          
  "Coba",                                  
  "Cocoyoc",                               
  "Coihaique", 
  "Coilaco",
  "Collons",
  "Colotlipa",                             
  "Comalapa",                              
  "Comba",
  "Combartseline",
  "Comberette",
  "Combire",
  "Combras",
  "Conchali", 
  "Condemines",
  "Corinth",
  "Corlay",
  "Corofin",
  "Coroi",
  "Corran",
  "Corwen",
  "Corzo",              
  "Couquelles",
  "Coutain",
  "Cozzera",
  "Crail",
  "Crapschalver",
  "Cremona",
  "Cresta",
  "Crestas",
  "Crieff",
  "Crispalt",
  "Cristallina",
  "Crocs",
  "Croisic",
  "Cromarty",
  "Crou",
  "Crusch",
  "Cuanlixco",    
  "Cuauhtemoc",   
  "Culm",
  "Cumbraes",
  "Cuncumen", 
  "Curacavi", 
  "Curepto", 
  "Curnera",
  "Curnius",
  "Curtagni",
  "Daarm Ben",
  "Dabu",
  "Dadunil",
  "Daigra",
  "Daingean",
  "Dangriga",                              
  "Darhan", 
  "Darm",
  "Decca",
  "Denopeste",
  "Derron",
  "Derwent",
  "Desriritsi",
  "Detgubehe",
  "Deveron",
  "Dezhou",
  "Diablerets",
  "Diemtigen",
  "Dionarap",
  "Dix",
  "Doedbygd",
  "Doettingen",
  "Dokur",
  "Domat",
  "Doramed",
  "Dornoch",
  "Dornod", 
  "Dorpodepe",
  "Dortiner",
  "Dozonodot",
  "Drammes",
  "Dremmer",
  "Drense",
  "Drimnin",
  "Drone",
  "Dros",
  "Drostobel",
  "Drrk K'cht",
  "Drumcollogher",
  "Drummore",
  "Dryck",
  "Drymen",
  "Dsitas",       
  "Dunbeath",
  "Duncansby",
  "Dunfanaghy",
  "Dunkeld",
  "Dunmanus",
  "Dunster",
  "Durness",
  "Duspondebe",
  "Duucshire",
  "Duvin",
  "Dzibilchaltun",
  "Edzna",        
  "Egern",
  "Eiger",
  "Elgomaar",
  "Ellesmere",
  "Ellon",
  "Elsig",
  "Ems",
  "Enfar",
  "Erdenet", 
  "Ergisch",
  "Erisort",
  "Ermitage",
  "Eskerfan",
  "Etoscha",
  "Etretat",
  "Ettrick",
  "Evolene",
  "Evouettes",
  "Faaa",
  "Fache",
  "Fanders",
  "Fanell",
  "Faner",
  "Fanromiful",
  "Fantanuglias",
  "Farafra",
  "Farandauz Girdl",
  "Fardun",
  "Farfustavu",
  "Fawumes",
  "Fcht'Myrrck",
  "Fecamp",
  "Fedeta Ko",
  "Ferbane",
  "Fernando Poo",
  "Ferpecle",
  "Fesevu",
  "Fessosado",
  "Fetlar",
  "Fewuke",
  "Fey",
  "Fezisit",
  "Fidaz",
  "Fimn Perg",
  "Fionnay",
  "Firgumi",
  "Fisis",
  "Fispas",
  "Flem",
  "Flims",
  "Flock",
  "Florina",
  "Fofen",
  "Foketan",
  "Fope",
  "Forcadona",
  "Fori",
  "Formby",
  "Fornd Perm",
  "Fotfantu",
  "Frainberg",
  "Frem Niim",
  "Fuhadit",
  "Fuorns",
  "Fural",
  "Furggeli",
  "Gaglianera",
  "Galloway",
  "Gamch",
  "Gampel",
  "Ganon",
  "Ganzhou",
  "Garina",
  "Garzora",
  "Gastginas",
  "Geal Charn",
  "Gefesvon",
  "Gehuzude",
  "Gelt Fiil",
  "Gemmi",
  "Gemna Toorn",
  "Genrebin",
  "Germindas Nim",
  "Gern Horonz",
  "Gerr",
  "Gersipa",
  "Gezukuda",
  "Ghirone",
  "Ghom",
  "Gidfod",
  "Giesen",
  "Gifford",
  "Girvan",
  "Gizat",
  "Glenagallagh",
  "Glenanane",
  "Glin",
  "Glion",
  "Glomera",
  "Glormandia",
  "Gluggby",
  "Gnackstein",
  "Gnoelhaala",
  "Go-Ne",
  "Golconda",
  "Gona Birm",
  "Goppen",
  "Gornen",
  "Gosolvusu",
  "Gotfawohu",
  "Gouille",
  "Gourock",
  "Govi-Altai", 
  "Graevbygd",
  "Gram Tzitsch",
  "Grandola",
  "Greina",
  "Gren Wiil",
  "Gresberg",
  "Gresir",
  "Greverre",
  "Grimentz",
  "Griminish",
  "Grisbygd",
  "Groddland",
  "Groix",
  "Gruben",
  "Grue",
  "Grur",
  "Guanajuato",
  "Guanoro",      
  "Gueida",
  "Guer",
  "Gufer",
  "Gugelun",
  "Guggernuell",
  "Guien-Gola",   
  "Guifer",
  "Guilvinec",
  "Gumm",
  "Gungg",
  "Guraletsch",
  "Gurkacre",
  "Gurnigel",
  "Hague",
  "Haikou",
  "Hainne",
  "Halkirk",
  "Handan",
  "Handegg",
  "Hasmerr",
  "Hatane",
  "Hauderes",
  "Helmsdale",
  "Helmsley",
  "Helsicke",
  "Helukadi",
  "Helvete",
  "Hentii", 
  "Heremence",
  "Herens",
  "Hipa",
  "Hipowi",
  "Hirehige",
  "Hoersalsveg",
  "Holbox",       
  "Holi",
  "Holopurtir",
  "Honakau",
  "Houat",
  "Hovd", 
  "Huamantla",                             
  "Huamuxtitlan",       
  "Huehuetenango",                         
  "Huitzo",                                
  "Huitzuco",           
  "Hullevala",
  "Hure",
  "Hyiida Venn",
  "Ickellund",
  "Iird Lerg",
  "Ill",
  "Illanz",
  "Ilz",
  "Imschlacht",
  "Inber",
  "Inverie",
  "Ipala",              
  "Irgh",
  "Irimba",             
  "Iserables",
  "Ixcamilca",    
  "Ixcoy",                                 
  "Ixtapantongo",       
  "Ixtatan",      
  "Izabal",             
  "Iztaccihuatl",       
  "Jaargh fycht",
  "Jable",
  "Jaca",
  "Jahrom",
  "Jeelash Myen",
  "Jeormel",
  "Jervbygd",
  "Jining",
  "Jonutal",                               
  "Jossalin",
  "Jotel",
  "Juchatengo",         
  "Juhaal Vriem",
  "Jungapeo",           
  "K'tn'krkt'pch",
  "Kaddervar",
  "Kallern",
  "Kanasin",            
  "Kander",
  "Kantunil",           
  "Karand",
  "Karegato",
  "Karkh Bream",
  "Karothea",
  "Kashmar",
  "Katakai",
  "Katn'pycht",
  "Kauraduna",
  "Kawanpi",
  "Kemmi",
  "Kenema", 
  "Kenuwu",
  "Keswick",
  "Kharsh'chrontp'n",
  "Khjycht Vrkt'n",
  "Kielder",
  "Kihotitul",
  "Killorglin",
  "Kina",
  "Kinbrace",
  "Kinchil",            
  "Kinibara",
  "Kintore",
  "Kirriemuir",
  "Kizepuku",
  "Kiziskuvo",
  "Kkr'gkh'kcht",
  "Klen",
  "Knesekt",
  "Kobbe",
  "Koblenz",
  "Koge",
  "Kohunlich",          
  "Koldu", 
  "Kole",
  "Kom",
  "Komarken",
  "Kossi",
  "Kovel",
  "Krck'tchkt",
  "Krk'kr'tch",
  "Krod",
  "Kufe",
  "Kursk",
  "La Morge",
  "La Sage",
  "Laard Fim",
  "Laax",
  "Labno",                                 
  "Lagos",
  "Lamlash",
  "Lana",
  "Langholm",
  "Lanzone",
  "Lapus",
  "Larache",
  "Larciolo",
  "Lareccio",
  "Larietta",
  "Larkanth",
  "Larmet",
  "Lauen",
  "Laurin",
  "Lautaro",
  "Lauvin",
  "Lavadignas",
  "Lavenzug",
  "Lavey",
  "Lavorceno",
  "Lehm Mitt",
  "Leighlin",
  "Lens",
  "Lervir",
  "Letfasvo",
  "Letse",
  "Leuk",
  "Leven",
  "Libigen",
  "Liblanggen",
  "Licata",
  "Limavady",
  "Lingen",
  "Lintan",
  "Lirec",
  "Liscannor",
  "Lisvanker",
  "Lobesu",
  "Locarno",
  "Lochalsh",
  "Lochcarron",
  "Lochinver",
  "Lochmaben",
  "Locmine",
  "Logama",
  "Lohner",
  "Lom",
  "Loriwoma",
  "Lorthalm",
  "Louche",
  "Louer",
  "Lrr'nn'cht",
  "Lubaantun",          
  "Luette",
  "Lugapa",
  "Lungi", 
  "Lurkabo",
  "Lusudtukul",
  "Luthiir",
  "Luvreu",
  "Luvrin",
  "Lybster",
  "Lynton",
  "Madpuliru",
  "Madun",
  "Magburaka",
  "Magordino",
  "Maighels",
  "Makeni", 
  "Maliens",
  "Mallaig",
  "Manferdi",
  "Mapastepec",   
  "Marcala",                               
  "Marcheggen",
  "Maredte",
  "Martigny",
  "Maruru",
  "Marzano",
  "Mata",
  "Matangi",
  "Mataro",
  "Mathon",
  "Matse",
  "Mauvoisin",
  "Mayennan",
  "Mayoux",
  "Mazembroz",
  "Medels",
  "Meina",
  "Melfi",
  "Melvaig",
  "Meneta Gai",
  "Menter",
  "Meretschia",
  "Merfe",
  "Merg Anab",
  "Merledaut",
  "Merlerault",
  "Meskawina",
  "Metail",
  "Methven",
  "Metra",
  "Metsch",
  "Mirlan",
  "Mirutta",
  "Miwa",
  "Mofakosa",
  "Moffat",
  "Mohn",
  "Moiry",
  "Momefoda",
  "Mon Pirg",
  "Monamolin",
  "Montana",
  "Monzon",
  "Moos",
  "Morasses",
  "Morella",
  "Morgel",
  "Mortenford",
  "Motot",
  "Motpan",
  "Motterascio",
  "Motu Tapu",
  "Mughels",
  "Muhi",
  "Mulagn",
  "Mulchic",            
  "Mullaghcarn",
  "Mulle",
  "Muota",
  "Murlins",
  "Murom",
  "Myrmadon Pyrch",
  "Naglens",
  "Nahuaha",            
  "Nahuala",      
  "Nair",
  "Nairn",
  "Nanchtitla",         
  "Nantais",
  "Nase",
  "Navenby",
  "Nax",
  "Nazis",
  "Nenday",
  "Nendaz",
  "Nepantla",                              
  "Nephin Beg",
  "Nestugele",
  "Netzahualcoyotl",    
  "Nezahualcoyotl",     
  "Nifadpuzer",
  "Nipa",
  "Niskby",
  "Noatu",
  "Nolla",
  "Nolle",
  "Nolrolko",
  "Noohm",
  "Nork",
  "Norlin",
  "Oarland",
  "Oeschinen",
  "Oey",
  "Ogata",
  "Ohcysp",
  "Olden",
  "Olenek",
  "Olg-Nuum",
  "Olinala",      
  "Olivone",
  "Oloron",
  "Omotana Ganete",
  "Oncedo",
  "Oorb Bo'oun",
  "Oorg",
  "Opanarm",
  "Orama",
  "Oranmore",
  "Orca",
  "Orchera",
  "Orgh Joon",
  "Orkh Pycht",
  "Orkha Pyr",
  "Ormgryte",
  "Orrebygd",
  "Orsieres",
  "Orvaux",
  "Otumba",       
  "Otutara",
  "Ouessant",
  "Ouna Byrch",
  "Oxkutzcab",          
  "Ozumba",       
  "Palmi",
  "Pan Gooma",
  "Panyu",
  "Papara",
  "Papea",
  "Parara",
  "Pardatsch",
  "Parilet",
  "Partry",
  "Pauer",
  "Paxila",                                
  "Pchh'Fkht'trchy",
  "Pendhon",
  "Penhalolen",
  "Penns",
  "Perkel",
  "Pernadalan Frieez",
  "Perski",
  "Petga",
  "Petlalcingo",        
  "Petorca", 
  "Piaenetsch",
  "Piaxtla",                               
  "Pidgan",
  "Pignia",
  "Pihe",
  "Pilcaya",      
  "Pinsec",
  "Piri Nai",
  "Pitetta",
  "Planggen",
  "Planken",
  "Platta",
  "Plattland",
  "Pleagne",
  "Pletschen",
  "Plouguerneau",
  "Plozevat",
  "Pochtenfall",
  "Pogelveir",
  "Polochic",                              
  "Ponchette",
  "Ponodamaddon",
  "Pontorson",
  "Pornichet",
  "Porteia",
  "Portein",
  "Portgeren",
  "Porthcawl",
  "Portimao",
  "Portrieux",
  "Potenza",
  "Pradasca",
  "Praestbygd",
  "Praflorie",
  "Pralong",
  "Preetsome",
  "Prelet",
  "Presu",
  "Prettstern",
  "Promischur",
  "Pryy T'cht",
  "Puchuncavi", 
  "Pudahuel",
  "Pupuya", 
  "Purcs",
  "Putla",        
  "Puzetta",
  "Pychtan Moor",
  "Quadra",
  "Queretaro",
  "Quetzalcoatl", 
  "Quezaltepeque",                         
  "Quiberon",
  "Quilicura", 
  "Quimistan",    
  "Quivolgo",
  "Raketo",
  "Ramoza",
  "Ranga",
  "Rantlu",
  "Rapel", 
  "Rappbygd",
  "Rasad",
  "Rascheu",
  "Raspominrel",
  "Rath Luire",
  "Raz",
  "Rechy",
  "Regeln Vraam",
  "Renards",
  "Rengg",
  "Retalhulen",         
  "Rethel",
  "Reulisse",
  "Rewewovu",
  "Riddes",
  "Riggenthorpe",
  "Risulene",
  "Riva",
  "Rivawiru",
  "Rive",
  "Rivu",
  "Rochfort",
  "Roddendor",
  "Rodels",
  "Rognero Pird",
  "Roin",
  "Rondadura",
  "Ronizisa",
  "Roptille",
  "Roter",
  "Rothenburg",
  "Ruchen",
  "Ruesch",
  "Rueve",
  "Ruinatsch",
  "Rungipiskon",
  "Rura",
  "Rutven",
  "Sabancuy",                              
  "Saekingen",
  "Safien",
  "Sagogn",
  "Sagunto",
  "Saillon",
  "Saklebille",
  "Salamanca", 
  "Salen",
  "Salums",
  "Samada Sura",
  "Samada Sut",
  "Samest",
  "Sandwick",
  "Sarab",
  "Sarkanvale",
  "Sarn",
  "Sarvenfesti",
  "Satarma",
  "Satrama",
  "Saxon",
  "Sayil",        
  "Scabuer",
  "Scaletta",
  "Scandamia",
  "Scarinish",
  "Scharboda",
  "Scharf",
  "Schwaren",
  "Schwiil",
  "Scourie",
  "Scuro",
  "Sefinen",
  "Sefninen",
  "Seibal",       
  "Seli",
  "Seninlatin",   
  "Serdan",                                
  "Serengasta",
  "Serengia",
  "Serov",
  "Sevgein",
  "Shanyin",
  "Siders",
  "Siegen",
  "Sierre",
  "Silvituc",                              
  "Simojovel",          
  "Sinan",
  "Sines",
  "Sinridi",
  "Sitten",
  "Skim",
  "Skokholm",
  "Skomer",
  "Skottskog",
  "Sledmere",
  "Somo",
  "Sorda",
  "Sorisdale",
  "Sosto",
  "Spakker",
  "Spannort",
  "Spegnet",
  "Spluegen",
  "Spunda",
  "St. Albin",
  "Stabbiello",
  "Stablets",
  "Stackforth",
  "Stafel",
  "Staffeln",
  "Staklesse",
  "Stavlatsch",
  "Stinchar",
  "Stoer",
  "Strichen",
  "Stroma",
  "Stugslett",
  "Suchixtepec",  
  "Sufers",
  "Suide",
  "Suke",
  "Summaprada",
  "Suretta",
  "Surpalis",
  "Suste",
  "Tabuk",
  "Tahatz",
  "Tahiri",
  "Takaroa",
  "Talganaciguaro",
  "Tamaulipas", 
  "Tamboa",
  "Tan-kah",                               
  "Tanintuhed",
  "Taranga",
  "Tarava",
  "Taro",
  "Tarraspan",
  "Tasro",
  "Tatarahapa",
  "Tau Tai",
  "Taunggyi",
  "Tautira",
  "Tawus",
  "Teahupo",
  "Tecun",              
  "Tegia Culm",
  "Tehuitzingo",  
  "Tekax",        
  "Tekik",        
  "Telchac",                               
  "Tendor",
  "Tenna",
  "Tepalitlan", 
  "Tepetlixpa",         
  "Terganom Bend",
  "Terri",
  "Tetuan",
  "Texcalyacac",  
  "Tezintlan",                             
  "Tgavrida",
  "Tharsis",
  "Thurso",
  "Thusis",
  "Thyon",
  "Tiare Taporo",
  "Tiarns",
  "Tiemcen",
  "Tikal",                                 
  "Tiksi",
  "Tilzapatla",         
  "Tingambato",                            
  "Tisu",
  "Tixkobob",                              
  "Tizimin",                               
  "Tlalmanalco",  
  "Tlalnepantla",                          
  "Tlangnistengo",                         
  "Tlatlaya",     
  "Tlayecac",           
  "Tobo",
  "Tolsta",
  "Tonrikerar",
  "Toppola",
  "Tor",
  "Torg Wisen",
  "Torrent",
  "Torridon",
  "Totlmaloya",   
  "Totolmajac",         
  "Tounot",
  "Tourla",
  "Tracuit",
  "Trapani",
  "Trebeuran",
  "Tregastel",
  "Treusch",
  "Trogne",
  "Tromeforth",
  "Trutten",
  "Tsan",
  "Tsava",
  "Tschingel",
  "Tsenzier",
  "Tsidjiori",
  "Tude",
  "Tudela",
  "Tudrowat",
  "Tujetsch",
  "Tuku",
  "Tula",               
  "Tulum",        
  "Tuma",
  "Tuminen",
  "Tuors",
  "Turia",
  "Turogeta",
  "Turtmann",
  "Tutsch Meel",
  "Tuzantla",     
  "Tzucacab",           
  "Ua Reka",
  "Uaxactun",           
  "Uezwil",
  "Uffiern",
  "Uganadarom-Luun",
  "Uh-May",                                
  "Uketa",
  "Uman",                                  
  "Ur-Grue",
  "Urganal",
  "Urgru",
  "Urmein",
  "Uurgh",
  "Uverhangai",
  "Uxelberg",
  "Vaddili",
  "Vaila",
  "Valdraus",
  "Valga",
  "Vallatscha",
  "Valognes",
  "Vals",
  "Vannes",
  "Vatsa",
  "Vauv",
  "Vefit",
  "Veisivi",
  "Veno",
  "Vercorin",
  "Verguin",
  "Vermala",
  "Vernlund",
  "Versam",
  "Veulden",
  "Vex",
  "Veysonnaz",
  "Vichiesso",
  "Victoria",
  "Videmanette",
  "Vikatebel",
  "Vilpesu",
  "Vimoutiers",
  "Vinu",
  "Virpiska",
  "Vissoie",
  "Vitg Dadens",
  "Viwizod",
  "Vollen",
  "Volovron",
  "Vons",
  "Vovadtapu",
  "Vovi",
  "Vrach",
  "Vusistelte",
  "Waeger",
  "Wahnen",
  "Waimer",
  "Walletsch",
  "Wamitit",
  "Weiningen",
  "Wengen",
  "Wengli",
  "Werd",
  "Wergenstein",
  "Wett",
  "Wettingen",
  "Wimedkivil",
  "Witten",
  "Witwil",
  "Wogoputa",
  "Wohaleko",
  "Wonpa",
  "Wonti",
  "Wovasadpo",
  "Wuddanebu",
  "Wugan",
  "Wuzewi",
  "Xalostoc",     
  "Xalpa",        
  "Xbacab",             
  "Xel-ha",             
  "Xicohtzingo",  
  "Xintetelco",                            
  "Xipetzingo",         
  "Xlapax",                                
  "Xmaben",       
  "Xolostoc",     
  "Xontormia",
  "Xort Piin",
  "Xpujil",       
  "Xunantunich",        
  "Yakleks",
  "Yalalag",      
  "Yalkabul",           
  "Yaxca",        
  "Yaxcaba",      
  "Yaxchilan",    
  "Yecapixtla",                            
  "Yorito",             
  "Yosonotu",                              
  "Ysosondua",          
  "Yuci",
  "Z'Tch'tk",
  "Zaalsehuur",
  "Zacapa",       
  "Zacatecas", 
  "Zaculeo",                               
  "Zamora",
  "Zapulla",
  "Zawis",
  "Zefedditgi",
  "Zerivebas",
  "Zevreila",
  "Ziiem",
  "Zikadsul",
  "Zinal",
  "Zingg",
  "Zitolfekor",
  "Zocac",              
  "Zodde",
  "Zodi",
  "Zoko",
  "Zontehuitz",   
  "Zorn",
  "Zosider",
  "Zudapo",
  "Zulosud",
  "Zuneu",
  "Zycht Fsst",
};

