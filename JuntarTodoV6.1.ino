                                            //COMENTAR TODOS LOS SERIALPRINTS PARA AHORRAR PROCESAMIENTO
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <string.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#endif

MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
//#define OUTPUT_READABLE_ACCELGYRO
//#define OUTPUT_BINARY_ACCELGYRO



///////////////////////////////////////////Declaraciones iniciales


typedef struct tabla_score {
  char nombre[8];
  int puntaje;
  int EEslot;
};

tabla_score  AllScores[10], NewScore;

int x = 16, y = 13, Direccion = 4;            //Direccion puede asumir 4 valores: 1 es arriba, 2 abajo, 3 iquierda y 4 derecha.
int DireccionAnterior = Direccion;            //La hago global, y que cambie en la función movimiento, pues la funcion sensor se llama cada 100 milis y movimiento cada 400.
int posx_Cabeza , posy_Cabeza;
int Perdiste = 0;                             //Si "Perdiste" es igual a 0 significa que el juego todavia está en curso.          
int longitud;
int manzanas_en_mapa = 0;
int CDF;                                      //contador de flujo, para ordenar el flujo de funciones, cual se ejecuta despues de cual
int Score;
int Mapa_N[16][13];

bool contador=0;

char Mapa[16][13];
String aaaa = "";

bool FT = 0;    //FT (First Time) al iniciar el juego es 0.
bool bandera_mov = 0;
bool bandera_timeout = 0;
bool inicializar = 0;
bool listoparainiciar = 0;
bool bandera_llenado_scores = 0;
bool opcionmando = 0;
bool sensorflag = 0;


Ticker velocidad_serpiente;
Ticker generar_manzanas;
//Ticker evento; comentado porque uso sensor
Ticker timeout;
Ticker lectura;



///////////////////////////////////////////Configuración de red y contraseña
const char *ssid = "WeMos Access Point"; // The name of the Wi-Fi network that will be created
const char *password = "politecnico";   // The password required to connect to it, leave blank for public network

//Objeto "servidor" en el puerto 80(puerto HTTP)
ESP8266WebServer server(80);



///////////////////////////////////////////Funciones WiFi
void banderatimeout() {//------------------------------------------Posibilita el Timeout
  bandera_timeout = 1;
  timeout.detach();
}

void ejecucion_timeout() { //--------------------------------------Timeout conexion: esperar a que se conecte automáticamente a la ultima red conocida
  bandera_timeout = 0;
  timeout.detach();

  if (WiFi.status() != WL_CONNECTED) {

    // para la toma de datos, la wemos funciona como un punto de acceso
    WiFi.softAP(ssid, password);             // Start the access point
    Serial.print("Access Point \"");
    Serial.print(ssid);
    Serial.println("\" started");

    Serial.print("IP address:\t");
    Serial.println(WiFi.softAPIP());         // Send the IP address of the Access Point of the ESP8266 to the computer
  }

  if (WiFi.status() == WL_CONNECTED) { //si ya esta conectado, igual crea el access point por si el cliente se olvida la IP.
    Serial.print("Conectado a red! direccion IP ESP -> ");
    Serial.print(WiFi.localIP()); //Imprime la IP local de ESP
    Serial.println("/home");

    WiFi.softAP(ssid, password);             // Start the access point
    Serial.print("Access Point \"");
    Serial.print(ssid);
    Serial.println("\" started");
    Serial.print("IP address:\t");
    Serial.println(WiFi.softAPIP());         // Send the IP address of the Access Point of the ESP8266 to the computer
  }
  Serial.println("Terminó ejecucion_timeout");
}

void cambiazo( String red, String contra) {//----------------------Cambia de punto de acceso a cliente, o de cliente en un WiFi a cliente en otro WiFi

  Serial.print("Conectando a ");
  Serial.println(red);

  WiFi.begin(red, contra); //empieza la conexión con el WiFi del hogar
  int a = 0;
  timeout.attach(20 , banderatimeout);  //cuando pasen 20 segundos, se prende una bandera global
  while (WiFi.status() != WL_CONNECTED) //espera una conexión
  {
    if ( a == 0) {
      Serial.print("Estableciendo conexión con... ");
      Serial.println(WiFi.SSID()); //Imprime el nombre de la red
      a = 1;
    }
    if (bandera_timeout == 1) {
      ejecucion_timeout(); //si pasaron 20 segundos y no se pudo conectar, crea el access point nuevamente
      break;
    }
    delay(599);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado a red! direccion IP ESP -> ");
    Serial.print(WiFi.localIP()); //Imprime la IP local de ESP
    Serial.println("/home");
  }
  Serial.println("termino cambiazo");
 //es importante que al conectarse correctamente a un WiFi, no desaparezca el Access Point, de
//lo contrario no podría mostrarse la nueva IP.
}

///////////////////////////////////////////Funciones de las páginas
String htmltomadatos() {//-----------------------------------------Una pagina inical donde están los formularios para conectarse o cambiar al WiFi

  String a;
  a = (server.arg("ssid").c_str());

  String b;
  b = (server.arg("contrasenia").c_str());

  String pamandartomadatos = "";
  pamandartomadatos += "   <!DOCTYPE html>\
<html>\
  <head>\
    <meta http-equiv='content-type' content='text/html; charset=UTF-8'>\
    <link rel='stylesheet'>\
    <title>Snake</title>\
    <style>\
    body {\
      background: #007F5F;\
      font-family:Courier New;\
      min-width: 772px;\
    }\
      \
    table, th, td {\
        border: 1px solid black;\
        border-collapse: collapse;\
      background: #f3f31e;\
    }\
      \
    th {\
        padding: 5px;\
        text-align: center;\
    }\
      \
    td {\
        padding: 5px;\
        text-align: center;\
      width: 50%;\
    }\
      \
      \
    #Contenedor {\
      margin: 0.5%;\
    }\
      \
    #Cabecera {\
      padding-top: 1%;\
      padding-bottom: 1.8%;\
      padding-right: 2.5%;\
      padding-left: 2.5%;\
      background-color: #1abc9c;\
      border-right: 1px solid;\
      border-left: 1px solid;\
      border-top: 1px solid;\
    }\
      \
    #Cuerpo {\
      background-color: #c5e61c;;\
      border: 1px solid;\
      font-size: 18px;\
      margin-bottom: 0px;\
      padding-bottom: 25.2%;\
    } \
\
    #Contenedor2{\
      float:right;\
      width:60%;\
      margin-right:3.5%;\
      border:1px solid;\
      display:inline-block;\
      border-radius: 8px;\
      padding: 0.4%;\
      background: #f3f31e;\
    }\
      \
    #Expl1{       \
      text-align:left;\
      margin-right:25%;                        \
    }\
    \
    #Expl2{       \
      text-align:left;                              \
    }\
      \
    #Form {        \
      float:right;\
      margin-right:3.5%;                \
    }\
      \
    #Scoreboard {\
      background: #1abc9c;\
      float: left;\
      margin-top: 4%;\
      margin-left: 3.5%;\
      margin-top: 2%;\
      border: 1px solid;\
      border-radius: 8px;\
      width: 26.8%;\
      minimo-width: 193px;\
      display: inline-block;\
    }\
      \
    #Contenedor3{\
      float: left;\
      margin-top: 3.5%;\
      margin-left: 5.1%;\
      border: 1px solid;\
      width: 60%;\
      display: inline-block;\
      border-radius: 8px;\
      padding: 0.4%;\
      background: #f3f31e;\
    }\
      \
    </style>\
  </head>\
  <body>\
    <div id='Contenedor'>\
      <div id='Cabecera'>\
        <h1> Snake </h1>\
      </div>\
      <div id='Cuerpo'>\
        <p>&nbsp;&nbsp; Bienvenido a: <i><b><u>Snake</u> <u>the</u> <u>game</u></b></i></p>\
        <br>\
        <p style='margin-left:5%;margin-right:5%'>Sólo por esta vez (o si cambias de WiFi) debes conectarte a la red WiFi de tu casa.</p>\
        <p style='margin-left:5%;margin-right:5%'>Si introduces erróneamente la red WiFi o contraseña, luego de veinte segundos puedes recargar esta misma página y volver a intentarlo. </p>\
        <br>\
        <div id='Contenedor2'>\
          <div id='Form'>\
            <form action='/' method='post'> <label for='ssid'>Nombre de red:</label><br>\
              <input id='ssid' name='ssid' type='text' autocapitalize='none'><br>\
              <br>\
              <form action='/' method='post'> <label for='contrasenia'>Contraseña:</label><br>\
              <input id='contrasenia' name='contrasenia' type='password'><br>\
              <br>\
              <input value='Aceptar' type='submit'> </form> </div>\
              <br>\
              <p id='Expl1' style='margin-right:40%'>En este formulario puede ingresar una red WiFi y su contraseña.</p>";


  if (a != "" && b != "") {
    cambiazo(a, b);
    pamandartomadatos += "<br> <pre>La nueva IP es " + WiFi.localIP().toString() + "/home</pre> \
    <p>Copia esa dirección IP. Luego, desconecta tu computadora o teléfono de la red WiFi 'WeMos Access Point', y conéctate a la red de tu casa.\
    Después, en un navegador, pega la nueva IP previamente copiada.</p>";
  }

  pamandartomadatos += "\
        </div>\
      </div>\
    </div>\
  </body>\
</html>";

  Serial.println("termino html toma de datos");
  return (pamandartomadatos);
}

String htmlHome() {//----------------------------------------------htmlHome
  //----------------String en la que esta la página /home del juego.
  int i = 0, j = 0;

  String pamandarhome = "";
  pamandarhome += "   <!DOCTYPE html>\
<html>\
  <head>\
    <meta http-equiv='content-type' content='text/html; charset=UTF-8'>\
    <link rel='stylesheet'>\
    <title>Snake</title>\
    <style>\
    body {\
      background: #007F5F;\
      font-family:Courier New;\
      minimo-width: 772px;\
    }\
      \
    table, th, td {\
        border: 1px solid black;\
        border-collapse: collapse;\
      background: #f3f31e;\
    }\
      \
    th {\
        padding: 5px;\
        text-align: center;\
    }\
      \
    td {\
        padding: 5px;\
        text-align: center;\
      width: 50%;\
    }\
      \
      \
    #Contenedor {\
      margin: 0.5%;\
    }\
      \
    #Cabecera {\
      padding-top: 1%;\
      padding-bottom: 1.8%;\
      padding-right: 2.5%;\
      padding-left: 2.5%;\
      background-color: #1abc9c;\
      border-right: 1px solid;\
      border-left: 1px solid;\
      border-top: 1px solid;\
    }\
      \
    #Cuerpo {\
      background-color: #c5e61c;;\
      border: 1px solid;\
      font-size: 18px;\
      margin-bottom: 0px;\
      height: 1000px;\
    } \
\
    #Contenedor2{\
      float:right;\
      width:60%;\
      margin-right:3.5%;\
      border:1px solid;\
      display:inline-block;\
      border-radius: 8px;\
      padding: 0.4%;\
      background: #f3f31e;\
    }\
      \
    #Expl1{       \
      text-align:left;\
      margin-left:1%;\
      margin-right:5%;\
      margin-top:1%\
    }\
    \
    #Expl2{       \
      text-align:left;                              \
    }\
      \
    #Form {        \
      float:right;\
      margin-right:3.5%;                \
    }\
      \
    #Scoreboard {\
      background: #1abc9c;\
      float: left;\
      margin-top: 4%;\
      margin-left: 3.5%;\
      margin-top: 2%;\
      border: 1px solid;\
      border-radius: 8px;\
      width: 26.8%;\
      minimo-width: 193px;\
      display: inline-block;\
    }\
      \
    #Contenedor3{\
      float: left;\
      margin-top: 3.5%;\
      margin-left: 5.1%;\
      border: 1px solid;\
      width: 60%;\
      display: inline-block;\
      border-radius: 8px;\
      padding: 0.4%;\
      background: #f3f31e;\
    }\
      \
    </style>\
  </head>\
  <body>\
    <div id='Contenedor'>\
      <div id='Cabecera'>\
        <h1> Snake </h1>\
      </div>\
      <div id='Cuerpo'>\
        <p>&nbsp;&nbsp; Bienvenido a: <i><b><u>Snake</u> <u>the</u> <u>game</u></b></i>\
        </p>\
        <!-- <p>&nbsp;&nbsp; Buena suerte! </p> -->\
        <div id='Contenedor2'>\
          <div id='Expl1'>Ingresa tu nombre y luego presiona en el botón para\
            comenzar el juego.</div>\
          <div id='Form'>\
            <form action='/home' method='post'> <label for='name'>Nombre:</label><br>\
              <input id='name' name='name' maxlength='7' type='text'  ><br>\
              <br>\
                <input value='Guardar nombre' type='submit'  >\
                </form>";

  if (aaaa != "") {

    pamandarhome += "<form action='/game' method='post'> <br>\
              <br>\
              <label><input type='radio' name='opcion' value='0'>Rectilíneo</label><br>\
              <label><input type='radio' name='opcion' value='1'>Angular</label><br>\
                <input value='Jugar' type='submit'  >\
                </form>";
    WiFi.softAPdisconnect(0); // una vez que llegaste a este punto, desconecta el AP para ahorrar recursos.
  }

  pamandarhome += "\
          </div>\
          ";
  if (aaaa != "") {
    pamandarhome += "<br><br><br><br><p>Ahora elige un modo de controlar el mando:</p>";
  }
  pamandarhome += "</div> <div id='Scoreboard'>\
          <table style='width:100%;margin-bottom: 18px;'>\
            <caption>Scoreboard</caption>\
            <tbody>\
              <tr>\
                <th>Nombre</th>\
                <th>Puntaje</th>\
              </tr>";

  tabla_score tabla_ayuda;

  for (i = 0; i < 10; i++) {      
    tabla_ayuda = AllScores[i];

    pamandarhome += "\
                <tr>\
                <td>" + String(tabla_ayuda.nombre) + "\
                </td>\
                <td>" + String(tabla_ayuda.puntaje) + "\
                </td>\
              </tr>";
  }

  pamandarhome += "\
            </tbody>\
          </table>\
        </div>\
        <div id='Contenedor3'>\
      <h3 style='margin-left:2.5%;margin-right:2.5%'> <u>About</u> <u>us</u> </h3>\
      <div id='Expl2'>\
              <p style=margin-left:5%;margin-right:5%'>El proyecto Wi-Snake está desarrollado por (de izquierda a derecha:) Uriel Salomón, Ariel Malvaso y Agustín Russo. Es nuestro trabajo final de la materia Microprocesadores, dictada en Sexto año del Instituto Politécnico\
              Superior de Rosario, por el profesor Yamil Montoya.</p>\
              <p style=margin-left:5%;margin-right:5%'> Consiste en el juego clásico de Snake, pero controlado por un sensor de movimiento (acelerómetro) y usando una placa WeMos para mostrarlo en una página web.\
              </p>\
              <div align='center'><img src='http://drive.google.com/uc?export=view&id=17MVl78jffvHaAF-oPPW_TBlz-E7x7W7e'\
              style='width: 466.8px; height: 367.2px;'  title='2020 fue un año raro'></div>\
      </div>\
        </div>\
      </div>\
    </div>\
  </body>\
</html>";


  Serial.println("termino htmlHome");
  return (pamandarhome);
}

String htmlGame() {//----------------------------------------------htmlGame
  //----------------String en la que esta la página /game del juego.
  int i = 0, j = 0;
  //Serial.println("");

  String pamandargame = "";
  pamandargame += "   <!DOCTYPE html>\
<html>\
  <head>";

  pamandargame += "\
  <meta http-equiv='content-type' content='text/html; charset=UTF-8'>\
    <link rel='stylesheet'>\
    <title>Snake</title>\
    <style>\
    body {\
      background: #007F5F;\
      font-family:Courier New;\
    }\
    #Contenedor {\
      margin: 0.5%;\
      background-color: #c5e61c;;\
      border: 1px solid;\
      font-size: 18px;\
      padding-bottom: 43%;\
    }      \
\
\
    #Contenedor2{\
      float: left;\
      width: 20%;\
      margin-left: 40%;\
      margin-top: 10%;\
      border: 1px solid;\
      display: inline-block;\
      border-radius: 8px;\
      padding: 0.4%;\
      background: #f3f31e;\
      text-align: center;\
    }   \
    \
    #Form {        \
      float:center;\
    }\
    #nombre{\
      float: right;\
      margin-right:2%;\
      margin-top:1%;\
      border: 1px solid;\
      display: inline-block;\
      border-radius: 8px;\
      padding: 0.4%;\
      background: #f3f31e;\
    }\
    \
    </style>";

/*
  if (Perdiste == 0) {
    //pamandargame += "<script type='text/javascript'> function actualizar(){window.location=window.location;} setInterval('actualizar()',401); </script>";
    //pamandargame +="<meta http-equiv='refresh' content='0.4'>";
    if (WiFi.status() != WL_CONNECTED) { //si está conectado como AP, no tiene conexión a internet para entrar a la libreria de JQuery
      pamandargame += "<meta http-equiv='refresh' content='0.4'>";
    }
  }
  */
  pamandargame += "\
  </head>\
                  <body>\
    <div id='Contenedor'>\
    <div id='nombre'>\
      <input id='name' name='name' maxlength='7' value=" + aaaa + " type='text'>\
      </div>\
      <div id='Contenedor2'>\
      ";


  
  if (Perdiste == 0) { //----------------Si no perdiste, te muestra la serpiente
    pamandargame += "<pre>";
    for (j = 0; j < y; j++) {
      for (i = 0; i < x; i++) {
        pamandargame += Mapa[i][j];
      }
      pamandargame += '\n';
    }
    pamandargame += "</pre>";
  }

  
  if (Perdiste == 1) {//------------Si perdiste, no te muestra la serpiente, sino un boton para regresar a /home
    pamandargame +=  "<form action='/home' method='post' style='text-align:center'> <label for='name'>Perdiste</label>\
              <br>\
              <p>Tu puntaje fue: " + String(NewScore.puntaje) + " </p>\
                <input id='Form' style='margin-bottom:3%' value='Ver Scoreboards' type='submit' >\
                </form>";
    pamandargame += "<form action='/game' id='Form' id='Form' method='post'> <br>\
                <input name='playagain' value='Jugar de nuevo' type='submit'  >\
                </form>";
  }

  //pamandargame += soyelmapa;   probar con string en vez de char
  //Serial.println(soyelmapa);

  pamandargame += "\
  </div>\
    </div>";


/*
 if (Perdiste==1 && contador==1) {
    Serial.println("entro al if de perdiste para volver a jugar");
    contador++;
    pamandargame += "<script type='text/javascript'> function actualizar(){window.location=window.location;}  </script>";
    //pamandargame += "<script type='text/javascript'> stop();</script>";
    
    }
*/
  
   if (contador==0) {
    //autorefresh de div
    contador=1;
    Serial.println("entro al if de set");
    //pamandargame += "<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js'></script>";
    // es la misma, pero capaz sacándola de otro lado va un poco mejor:
    pamandargame += "<script type='text/javascript' src='https://code.jquery.com/jquery-3.5.1.min.js'></script>";
    //otra opción sería probar poner la librería JQuery como string en la WeMos: Función String libreria(){} que devuelva la String con la libreria. en el src de pamandargame llamar a esa función.
    //tiene 4330 caracteres de largo la libreria
    //pamandargame += "<script type='text/javascript' src='"+libreria()+"'></script>"; tampoco funciono pues la libreria contiene el caracter "\", que arduino no lo acepta. además tira otros errores de compilacion

    //pamandargame += "<script type='text/javascript' src='file:///C:/Users/Ariel/Documents/libjquery.js'></script>"; no funciono
    
    //conclusiones: lo que realmente refresca la pagina es el .load, lo otro no sirve si no apretas F5. ademas con cualquier division que pongas en el load lo va a recargar igual.
      //el setinterval no tiene que ver con las funciones del arduino: una vez que esta subida a la pagina, se sigue ejecutando hasta que se detenga el intervalo o desaparezca del html.
   
   //en document ready, funciono bien var t = setTimeout(detener,10000);\ 
    //pamandargame += "<script type='text/javascript'>\
    $(document).ready(function(){\
                        \
                        \
   $.ajaxSetup({ cache: false });\
 });\
    var intervalo = setInterval(refrescar,380);\
    function refrescar() {\
  $('#Contenedor2').load(window.location.href + ' #Contenedor2 > *');\
}\
\
    function detener() { \
            clearInterval(intervalo); \
        }\
        \
</script>";


//pamandargame += "<script type='text/javascript'> function actualizar(){location.reload(true);} setInterval('actualizar()',390); </script>";    


  pamandargame += "\
  <script type='text/javascript'> \
  $(document).ready(function(){\
  \
  var intervalo = setInterval(function(){\
      $('#Contenedor2').load(window.location.href + ' #Contenedor2 > *' );\
  }, 380); \
  });\
  </script>";


    //decime que sí           setInterval('refrescar()',400);\  
    //pamandargame += "<script type='text/javascript' >\
    $(document).ready(function(){\
    refrescar();\
   $.ajaxSetup({ cache: false });\
 });\
    function refrescar() {\
  $('#Contenedor2').html('";

        //pamandargame += "<pre>"+ WiFi.localIP().toString() +"</pre>"; //esto anda, pues no tiene ni '\n', ni chars en vez de String
/*  
  pamandargame +="<pre>";
  for (j = 0; j < y; j++) {
      for (i = 0; i < x; i++) {
        pamandargame += String(Mapa[i][j]);
      }
      pamandargame += "<br>";
    }
    pamandargame += "</pre>";
    
    
  pamandargame += "');\
}</script>";

*/
       
                                                                          //esta tiene que ser
        /*
    pamandargame += "<script type='text/javascript'>\
        function actualizar(){$('#Contenedor2').html('";
         pamandargame +="<pre>";
    for (j = 0; j < y; j++) {
      for (i = 0; i < x; i++) {
        pamandargame += String(Mapa[i][j]);
      }
      pamandargame += "<br>";
    }
    pamandargame += "</pre>";
    
        pamandargame += "')}window.setInterval('actualizar()',401); </script>";
*/

      
    /*
    //otro metodo   
        pamandargame +="<script type='text/javascript'>\
        $(document).ready(function(){\
         $('#Contenedor2').html('";
    pamandargame +="<pre>";
    for (j = 0; j < y; j++) {
      for (i = 0; i < x; i++) {
        pamandargame += String(Mapa[i][j]);
      }
      pamandargame += "<br>";
    }
    pamandargame += "</pre>";
    
        pamandargame += "');\
        setInterval(function() {\
                $('#Contenedor').load('#contenedor2');\
            }, 3000);\
        });\
      \
      </script>";
    */

    //otro metodo, solo para poner algo nuevo pero no dinamico, o para que sea dinamico tengo que poner variables
    //pamandargame+="<script type='text/javascript'>function render (){$('#Contenedor2').html('algo')}window.setInterval(render,401);</script>";

   }

  

  pamandargame += "\
   </body>\
  </html>";

  Serial.println("termino htmlGame");
  
  return (pamandargame);
}

void handletomadatos() {//-----------------------------------------Manejo tomadatos
  server.send(200, "text/html", htmltomadatos());
  Serial.println("termino pagina toma de datos");
}

void handleHome() {//----------------------------------------------Manejo /home
  aaaa = (server.arg("name").c_str());
  Serial.println(aaaa); //imprime el nombre en string
  opcionmando = (server.arg("opcion"));
  Serial.println(server.arg("opcion"));
  if (aaaa != "") {
    //aaaa.toCharArray(NewScore.nombre, aaaa.length());//-------Guardo el nombre ingresado en NewScore
    aaaa.toCharArray(NewScore.nombre, 8);                        //ENTRAN SOLO 7 POR EL MAXLENGHT. Pero quizas un caracter sea el '\0'.
    listoparainiciar = 1;
    contador=0;
  }
  Serial.println(NewScore.nombre);//imprime el nombre en char
  server.send(200, "text/html", htmlHome());
  Serial.println("termino handleHome");
}

void handleGame() {//----------------------------------------------Manejo /game
  if (listoparainiciar == 1) {
    inicio();
    listoparainiciar = 0;

    if (server.arg("opcion") == "0")//ojo, los datos recogidos son Strings, no son numeros
      opcionmando = 0;
    if (server.arg("opcion") == "1")
      opcionmando = 1;
  }
   
  if (server.arg("playagain") == "Jugar de nuevo"){
    listoparainiciar=1;
    contador=0;
    Serial.println("entro al if playagain");
    }
  server.send(200, "text/html", htmlGame());
  Serial.println("termino handleGame");
}

void setup() {//---------------------------------------------------Setup
  Serial.begin(115200);
  delay(1000);
  EEPROM.begin(512);
  server.begin(); //Inicia el servidor
  Serial.println("empieza setup");

  WiFi.mode(WIFI_AP_STA); //Declara a la WeMos simultáneamente en modos Station y Access Point
  //Serial.println(WiFi.status());
  //Serial.println(WL_CONNECTED); vale 3

  timeout.attach(4, ejecucion_timeout); //espera 4 segundos a ver si ya esta conectado

  //Crea una asociación entre la direccion web y las funciones que seran utilizadas en el sketch
  server.on("/home", handleHome);  //// Manejo el ingreso de datos del html
  server.on("/game", handleGame);
  server.on("/", handletomadatos);


  //sensor
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  //hay un ejemplo en el otro programa para cambiar el offset

  Serial.println("termino setup");
}

void loop() {//----------------------------------------------------Loop
  //----------------Funcionamiento ordenado del programa. Primero se procede a la lectura del sensor cada 100ms, luego, al iniciar la placa, se llena la estructura
  //----------------AllScores con los datos de la EEPROM. Una vez realizadas esas cosas en caso de que el juego esté a punto de comenzar, se procederá al curso
  //----------------normal del loop paso por paso.
  server.handleClient();//Analiza las solicitaciones via web

  if(sensorflag==1){
    sensorflag=0;
    sensor();
    }
  
  if (bandera_llenado_scores == 0) { //---------------------------------Apenas prenda la WeMos se ordena la scoreboard
    llenado_AllScores();
    table_sorting();
    bandera_llenado_scores = 1;
  }

  if (inicializar == 1) {
    switch (CDF) {
      case 0: {
          if (Perdiste == 1) {
            NewScore.puntaje = Score;
            comparacion_nuevo_score();
            llenado_AllScores();
            table_sorting();
            FinJuego();
          }
          marco();
          CDF++;
          break;
        }
      case 1: {
          nuevo_juego();
          CDF++;
          break;
        }
      case 2: {
          generacion_manzana();
          CDF++;
          break;
        }
      case 3: {
          conversor_numero_a_caracter(FT);
          FT = 1;
          Serial.println("Nuevo juego inicializado");
          //PrintMapa();
          CDF++;
          break;
        }

      //-------------------------Funcionamiento normal
      case 4: {
          if (bandera_mov == 1) {
            
            bandera_mov = 0;
            //Serial.print("\t \t \t \t \t \t \t \t \t "); Serial.println(Direccion);
            movimiento();
          }
          // CDF avanza dentro de movimiento
          break;
        }
      case 5: {
          conversor_numero_a_caracter(FT);
          CDF++;
          break;
        }
      case 6: {
          PrintMapa();
          CDF++;
          break;
        }
      case 7: {
          //Serial.println("caso 7 cdf");

          //htmlGame();//-----Se actualiza la pagina, y el autorefresh se encarga de mostrar, por eso no llamo a handle
          //handleGame();//probando
          CDF++;
          break;
        }
      case 8: {
          CDF = 4;
          break;
        }
      default:
        Serial.println ("default del switch");
    }
  }
}

///////////////////////////////////////////Funciones del juego propiamente dicho
void marco () {//--------------------------------------------------Marco: LLena la matriz Mapa_N con números.
  //----------------Función encargada de llenar la matriz numérica Mapa_N[] con números dependiendo de la ubicación para luego ser trabajada a lo largo del programa
  //Serial.println("marco inicio");
  for (int i = 0; i < x ; i++) {
    for (int n = 0; n < y ; n++) {
      if (i == 0) {
        Mapa_N[i][n] = -2;
        if (n == 0)Mapa_N[i][n] = -3;
        if (n == (y - 1))Mapa_N[i][n] = -4;
      }
      else if (i == (x - 1)) {
        Mapa_N[i][n] = -2;
        if (n == 0)Mapa_N[i][n] = -5;
        if (n == (y - 1))Mapa_N[i][n] = -6;
      }
      else if (n == 0 ) {
        Mapa_N[i][n] = -2;
      }
      else if (n == (y - 1)) {
        Mapa_N[i][n] = -2;
      }
      else
        Mapa_N[i][n] = -1;
    }
  }
  Serial.println("marco final");
}

void movimiento() {//----------------------------------------------Función principal del snake.
  //----------------Acá se comienza analizando en que sentido se moverá el jugador y si es que se choca, come una manzana o se mueve a un espacio vacío. 
  //----------------Dependiendo de eso ingresará a una función determinada con variables definidas.
  //Serial.println(Direccion);

  //Serial.println("Movimiento inicio");
  int llegoacod;
  int ayudaX = posx_Cabeza;
  int ayudaY = posy_Cabeza;
  CDF++;

  switch (Direccion) {
    case 1: {//-------------------------------------------El jugador decide mover la serpiente hacia arriba.
        llegoacod = Mapa_N[posx_Cabeza][posy_Cabeza - 1]; // me muevo arriba
        Mapa_N[posx_Cabeza][posy_Cabeza] = 2;
        Mapa_N[posx_Cabeza][--posy_Cabeza] = 1;
        break;
      }
    case 2: {//-------------------------------------------El jugador decide mover la serpiente hacia abajo.
        llegoacod = Mapa_N[posx_Cabeza][posy_Cabeza + 1]; // me muevo abajo
        Mapa_N[posx_Cabeza][posy_Cabeza] = 2;
        Mapa_N[posx_Cabeza][++posy_Cabeza] = 1;
        break;
      }
    case 3: {//-------------------------------------------El jugador decide mover la serpiente hacia la izquierda.
        llegoacod = Mapa_N[posx_Cabeza - 1][posy_Cabeza]; // me muevo a la izquierda
        Mapa_N[posx_Cabeza][posy_Cabeza] = 2;
        Mapa_N[--posx_Cabeza][posy_Cabeza] = 1;
        break;
      }
    case 4: {//-------------------------------------------El jugador decide mover la serpiente hacia la derecha.
        llegoacod = Mapa_N[posx_Cabeza + 1][posy_Cabeza]; // me muevo a la derecha
        Mapa_N[posx_Cabeza][posy_Cabeza] = 2;
        Mapa_N[++posx_Cabeza][posy_Cabeza] = 1;
        break;
      }
  }

  if (llegoacod == -2 || llegoacod > 2) {//--------------El jugador choca y pierde.
    //pongo mayor a 2 porque la unica forma de que pierda topándose con 2, es cambiando de sentido, cosa que
    //es imposible por lo codeado en la funcion sensor
    Serial.print("Perdiste en funcion movimiento, llegoacod igual a "); Serial.println(llegoacod);
    Perdiste = 1;
    CDF = 0;
    generar_manzanas.detach();
    generar_manzanas.attach(8 , generacion_manzana);
  }
  else {
    if (llegoacod == 0) {
      Score += 100;
    }
    movimiento_numeros(ayudaX, ayudaY, llegoacod);//------La partida sigue su curso.
  }
  DireccionAnterior = Direccion;
  //return; ponerlo en la ultima linea es lo mismo a no ponerlo, creo
}

void pregunta_manzana() {//----------------------------------------Se fija si hay manzanas en el mapa.
  //----------------Revisa la cantidad de manzanas en el mapa, es encargada de generar una manzana en el caso de que no haya y de que el jugador pierda si llega a 10 sin comer.
  if (manzanas_en_mapa == 0) generacion_manzana();
  if(manzanas_en_mapa >= 10) {Perdiste=1; FinJuego();}
}

void generacion_manzana() {//--------------------------------------Genera y ubica una manzana en el mapa.
  //----------------Genera una manzana localizando un lugar del mapa en el que no esté la serpiente ni una manzana, ni el borde del mapa, en caso de que se genere un par de coordenadas 
  //----------------en las que hay algo se procederá a una recursión de la función.
  //Serial.println("Generando manzana");
  int posx_nueva_manzana = random(1, x - 1);
  int posy_nueva_manzana = random(1, y - 1);
  if (Mapa_N[posx_nueva_manzana][posy_nueva_manzana] == -1) {
    Mapa_N[posx_nueva_manzana][posy_nueva_manzana] = 0;
    manzanas_en_mapa++;
  }
  else {
    generacion_manzana();
  }
  //Serial.println("Manzana generada");
}

void inicio() {//--------------------------------------------------Inicializa el juego
  //----------------Prepara el juego para comenzar a funcionar, inicializa los tickers y define las variables. 
  generar_manzanas.attach(8 , generacion_manzana);
  velocidad_serpiente.attach_ms(400 , bandera);
  //evento.attach_ms(100 , serialEvent);
  lectura.attach_ms(100 , banderasensor);
  CDF = 0;
  inicializar = 1;
  Perdiste = 0;
  Serial.println("termino inicio serpiente");
}

void FinJuego() {//------------------------------------------------Concluye el juego
  //----------------Finaliza el juego apagando todos los tickers y limpia el mapa.
  inicializar = 0;
  //evento.detach();
  generar_manzanas.detach();
  velocidad_serpiente.detach();
  lectura.detach();
  marco();    //lo puse para que no se muestre el ultimo frame de la partida anterior
  conversor_numero_a_caracter(FT);
}

void nuevo_juego() {//---------------------------------------------Al iniciar o perder el juego se reinician algunos valores. Podría incluirse en otras funciones pero, como dijo Nietzsche: "Si funciona, no toqués más".
  //----------------Reposiciona la serpiente para comenzar nuevamente y reinicia unas variables.
  Serial.print("Tu score es de: ");
  Serial.print(Score);
  Serial.println(" points");
  Score = 0;
  Perdiste = 0;
  Direccion = 4;
  FT = 0;
  manzanas_en_mapa = 0;
  longitud = 3;
  posx_Cabeza = (x / 2) - 5;
  posy_Cabeza = (y / 2) - 1;
  int posx_Cola = (x / 2) - 7;
  int posy_Cola = (y / 2) - 1;
  int posx_guia = (x / 2) - 6;
  int posy_guia = (y / 2) - 1;
  Mapa_N[posx_Cabeza][posy_Cabeza] = 1;
  Mapa_N[posx_guia][posy_guia] = 2;
  Mapa_N[posx_Cola][posy_Cola] = 3;
}

void movimiento_numeros(int ayudaX, int ayudaY, int llegoacod) {//-Lee la matriz Mapa_N y realiza el movimiento de la serpiente.
  //----------------Localiza cada cuerpo de la serpiente y mueve uno por uno utilizando los números de la matriz Mapa_N[], recordar que 1 es la cabeza y los numeros mayores a 2 son cuerpos.
  //----------------Luego procede a revisar si en el último movimiento realizado se comió o no una manzana y decide que se hará.
  int i = 0;
  //Serial.println("Moviendo numeros");
  for (i = 2 ; i <= longitud ; i++) {
    if (Mapa_N[ayudaX][ayudaY - 1] == i) {//--------------Chequeo casillero de arriba.
      Mapa_N[ayudaX][--ayudaY] = i + 1;
    }
    else if (Mapa_N[ayudaX][ayudaY + 1] == i) {//---------Chequeo casillero de abajo.
      Mapa_N[ayudaX][++ayudaY] = i + 1;
    }
    else if (Mapa_N[ayudaX - 1][ayudaY] == i) {//---------Chequeo casillero de la izquierda.
      Mapa_N[--ayudaX][ayudaY] = i + 1;
    }
    else if (Mapa_N[ayudaX + 1][ayudaY] == i) {//---------Chequeo casillero de la derecha.
      Mapa_N[++ayudaX][ayudaY] = i + 1;
    }
    else Serial.println("Error en movimiento numero");
  }
  switch (llegoacod) {
    case -1: {//------En caso de que no coma una manzana la cola se mueve.
        Mapa_N[ayudaX][ayudaY] = -1;
        break;
      }
    case 0: {//-------En caso de que coma una manzana la longitud aumenta.
        manzanas_en_mapa--;
        longitud++;
        break;
      }
    default: {
        Perdiste = 1;
        Serial.println("perdiste en movimiento numeros");
        CDF = 0;
        generar_manzanas.detach();
        generar_manzanas.attach(8 , generacion_manzana);
        break;
      }
  }
  pregunta_manzana();
  //Serial.println("Movimiento completado");
}

void conversor_numero_a_caracter(int FT) {//-----------------------Utiliza los números de la matriz Mapa_N y los reemplaza con caracteres en la matriz Mapa.
  //----------------Almacena una versión convertida de la matriz Mapa_N[] en Mapa[]. Esa versión constará de caracteres para que sea más bonito a la vista del jugador.
  //----------------Recordando que -1 son espacios en blancos, 0 son manzanas, 1 es la cabeza de la serpiente , de 2 para arriba son cuerpos y de -2 para abajo es el borde del mapa.
  //soyelmapa = "";
  for (int i = FT ; i < x - FT  ; i++) {
    for (int n = FT ; n < y - FT  ; n++) {
      switch (Mapa_N[i][n]) {
        case -1: {//--En caso de (-1), se deja un espacio en Mapa[][].
            Mapa[i][n] = ' ';
            //soyelmapa += " ";
            break;
          }
        case 0: {//---En caso de (0), se ubica una "O", la cual simboliza una manzana.
            //Mapa[i][n] = '■';
            //soyelmapa += "■";
            Mapa[i][n] = 'O';
            break;
          }
        case 1: {//---En caso de (1), se ubica la cabeza en un sentido en específico.
            switch (Direccion) {
              case 1: {//--------------Arriba.
                  Mapa[i][n] = '^';
                  //soyelmapa += "^";
                  break;
                }
              case 2: {//--------------Abajo.
                  //Mapa[i][n] = '∨';
                  //soyelmapa += "∨";
                  Mapa[i][n] = 'v';
                  break;
                }
              case 3: {//--------------Izquierda.
                  Mapa[i][n] = '<';
                  //soyelmapa += "<";
                  break;
                }
              case 4: {//--------------Derecha.
                  Mapa[i][n] = '>';
                  //soyelmapa += ">";
                  break;
                }
              default: {//-------------Error.
                  Mapa[i][n] = '@';
                  //soyelmapa += "@";
                  break;
                }
            }
            break;
          }//---------En estos siguientes casos, Mapa[][] escribe los bordes del mapa.
        case -3: {//--En estos siguientes 5 casos se ubica el marco del Mapa.
            //Mapa[i][n] = '╔';
            //soyelmapa += "╔";
            Mapa[i][n] = '#';
            break;
          }
        case -4: {
            //Mapa[i][n] = '╚';
            //soyelmapa += "╚";
            Mapa[i][n] = '#';
            break;
          }
        case -5: {
            //Mapa[i][n] = '╗';
            //soyelmapa += "╗";
            Mapa[i][n] = '#';
            break;
          }
        case -6: {
            //Mapa[i][n] = '╝';
            //soyelmapa += "╝";
            Mapa[i][n] = '#';
            break;
          }
        case -2: {
            if (Mapa_N[i][n] == -2 && (i == 0 || i == (x - 1) ) ) {
              //Mapa[i][n] = '║';
              //soyelmapa += "║";
              ////soyelmapa += '\n';
              Mapa[i][n] = '|';
            }
            else { //Mapa[i][n] = '═';
              //soyelmapa += "═";
              Mapa[i][n] = '-';
            }
            break;
          }
        default: {//--Finalmente se ubica el cuerpo de la Snake.
            Mapa[i][n] = '*';
            //soyelmapa += "*";
            break;
          }
      }
    }
  }
}

void bandera() {//-------------------------------------------------Función que permite el funcionamiento correcto de la función movimiento y el CDF (Contador de Flujo).
  //----------------Al activarse esta bandera se permitirá a la serpiente moverse un casillero.
  bandera_mov = 1;
}

void PrintMapa() {//-----------------------------------------------Muestra el juego en el monitor serial. Usada solamente para debuggear.
  for (int z = 0; z < y ; z++) {
    for (int i = 0; i < x ; i++) {
      Serial.print(Mapa[i][z]);
      //Serial.print("/");
    }
    Serial.println("");
  }
}

void serialEvent() {//---------------------------------------------Fue usada para direccionar la serpiente en una primera instancia, antes de utilizar el sensor.
  
  if (Serial.available()) {  //Lee el primer caracter (del buffer)y sigue de largo.
    // get the new byte:
    char inChar = (char)Serial.read();
    //Serial.println(inChar);
    int DireccionAnterior = Direccion;
    //Direccion = inChar;

    switch (inChar) {
      case 'w': {
          Direccion = 1;
          break;
        }
      case 's': {
          Direccion = 2;
          break;
        }
      case 'a': {
          Direccion = 3;
          break;
        }

      case 'd': {
          Direccion = 4;
          break;
        }
    }
    if ((DireccionAnterior == 1 && Direccion == 2) || (DireccionAnterior == 2 && Direccion == 1) || (DireccionAnterior == 3 && Direccion == 4) || (DireccionAnterior == 4 && Direccion == 3)) {
      Direccion = DireccionAnterior;
      //Serial.println("Dirección inválida.");
    }

  }
}

///////////////////////////////////////////Funciones EEPROM
void comparacion_nuevo_score() {//---------------------------------Compara el Score nuevo con los almacenados y reemplaza los valores pertinentes.
  //----------------Una vez finalizada la partida esta función revisa, primero, si el nombre de usuario ya aparece en la Scoreboard. Si así fuera, y el puntaje es mayor al anterior de ese mismo usuario, actualiza el valor del puntaje.
  //----------------Si el nombre de usuario no aparece en la Scoreboard, analiza si el puntaje alcanzado por el jugador es mayor a alguno de los almacenados.
  //----------------En caso de que asi sea, se reescribe la posición de la EEPROM del menor puntaje con los nuevos datos.
  int filas = 10;
  int minimo = 15400;
  int Slot = 200;

  for (int i = 0 ; i < filas ; i++) {
    if (strcmp(NewScore.nombre, AllScores[i].nombre)  == 0) { //---------Son iguales.
      if (NewScore.puntaje > AllScores[i].puntaje) { //------------------El score es mayor al anterior, reemplazo el score viejo por el nuevo.
        Slot = AllScores[i].EEslot;
        NewScore.EEslot = Slot;
        AllScores[Slot] = NewScore;
        EEPROM.put(NewScore.EEslot * sizeof(tabla_score) , NewScore);
        EEPROM.commit();
      }
      minimo = 15400;//-----------Sale del for y no entra al if despues del for.
      break;
    }
    else if (AllScores[i].puntaje < minimo) {
      minimo = AllScores[i].puntaje;
      Slot = AllScores[i].EEslot;
    }
  }
  if (NewScore.puntaje > minimo) {
    NewScore.EEslot = Slot;
    AllScores[Slot] = NewScore;//-------------------------------¡¡¡¡¡¡¡¡¡¡¡¡¡ESTOY LOCO??????????????????????
    EEPROM.put(NewScore.EEslot * sizeof(tabla_score) , NewScore);//-----NO SE COMO FUNCA LO DE ARRIBAAAAAAAAAA
    EEPROM.commit();
  }
}

void llenado_AllScores() {//---------------------------------------Llena la matriz de structs con los datos de la EEPROM.
  int filas = 10;
  //Serial.println("All eeprom:");
  for (int x = 0 ; x < filas ; x++) {
    EEPROM.get( x * sizeof(tabla_score) , AllScores[x]);
  }
}

void table_sorting() {//-------------------------------------------Ordena la matriz de structs.
  //--------------------Ordena la estructura de puntajes y nombres mediante el método de ordenamiento de burbuja, utilizando como variable de comparación el puntaje. 
  bool b = true;
  int filas = 10;
  tabla_score score_temp;

  while (b == true) { //------------------------------------Bucle que se ejecuta mientas ocurra un cambio de posicion en los datos
    b = false;
    for (int i = 0 ; i <  filas - 1; i++) { //Si hay 10 elementos, hago 9 comparaciones
      if (AllScores[i + 1].puntaje > AllScores[i].puntaje) {
        b = true;//-----Hay un cambio de posicion
        score_temp = AllScores[i];
        AllScores[i] = AllScores[i + 1]; //-----------------Intercambio de la posicion de los datos
        AllScores[i + 1] = score_temp;
      }
    }
  }
}

void sensor() {//--------------------------------------------------Direcciona la serpiente, de acuerdo con el modo de juego elegido. 

  //Serial.print("\t \t \t \t \t \t \t \t \t empieza sensor "); Serial.println(Direccion);
  //String dirpalabra = "";

  if (opcionmando == 0) { //si quisiera hacerlo cambiar la direccion moviendo en forma lineal:
    //no hace falta que sea brusco el movimiento, sino que se mantenga en el tiempo más de 100 milisegundos
    //Serial.print("opcion mando:"); Serial.println(opcionmando);
    accelgyro.getRotation(&gx, &gy, &gz);

    if ((abs(gz) < 12000) && (abs(gx) < 12000) ) {
      return;//para ahorrar recursos pongo un return en cada if, para que una vez que obtuvo la dirección, termine la funcion.
    }
    if (gz > 12000 && gz > abs(gx)) {
      //dirpalabra="izquierda";
      if ( DireccionAnterior != 4) {
        Direccion = 3;
      }
      return;
    }
    if (gz < -12000 && abs(gz) > abs(gx) ) {
       //dirpalabra="derecha";
      if ( DireccionAnterior != 3) {
        Direccion = 4;

      }
      return;
    }
    if (gx < -12000 && abs(gx) > abs(gz)) {
      //dirpalabra="abajo";
      if ( DireccionAnterior != 1) {
        Direccion = 2;

      }
      return;
    }
    if (gx > 12000 && gx > abs(gz)) {
       //dirpalabra="arriba";
      if ( DireccionAnterior != 2)
      { Direccion = 1;

      }
      return;
    }

  }

  if (opcionmando == 1) { //si quisiera hacerlo cambiar la direccion manteniendo un angulo:
    //Serial.print("opcion mando:"); Serial.println(opcionmando);
    accelgyro.getAcceleration(&ax, &ay, &az);
    if ((abs(ay) < 10000) && (abs(ax) < 10000) ) {
      return;
    }
    if (ax > 10000 && ax>abs(ay)) {
     //dirpalabra="izquierda";
      if ( DireccionAnterior != 4) {
        Direccion = 3;
      } return;
    }

    if (ax < -10000 && abs(ax)>abs(ay)) {
    //dirpalabra="derecha";
      if ( DireccionAnterior != 3) {
        Direccion = 4;
      }
      return;
    }
    if (ay > 10000 && ay>abs(ax)) {
  //dirpalabra="arriba";
      if ( DireccionAnterior != 2) {
        Direccion = 1;
      }
      return;
    }
    if (ay < -10000 && abs(ay)>abs(ax)) {
      //dirpalabra="abajo";
      if ( DireccionAnterior != 1) {
        Direccion = 2;
      }
      return;
    }
  }
  //Serial.print("\t \t \t \t \t \t \t \t \t termina sensor "); Serial.println(Direccion);
}

void banderasensor(){//--------------------------------------------Posibilita la lectura cada cierto período de tiempo de los datos del sensor.
  sensorflag=1;
  }

  
