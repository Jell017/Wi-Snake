//Cosas por hacer: HANDLES, HTML´s, EEPROM: WRITE, READ, SORT.

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <string.h>

Ticker aver;


typedef struct tabla_score {
  char nombre[8];
  int puntaje;
};

//------------------------------------------------------------------------------------Declaraciones iniciales
int x = 16, y = 13, Direccion = 4;            //Direccion puede asumir 4 valores: 1 es arriba, 2 abajo, 3 iquierda y 4 derecha.
int posx_Cabeza , posy_Cabeza;
int Perdiste = 0;                             //Si "Perdiste" es igual a 0 significa que el juego todavia está en curso.
int FT = 0;                                   //FT (First Time) al iniciar el juego es 0.
int longitud;
int manzanas_en_mapa = 0;
int CDF;
char Mapa[16][13];
//String soyelmapa ="";
int Mapa_N[16][13];
int bandera_mov = 0;
int Score;
Ticker velocidad_serpiente;
Ticker generar_manzanas;
Ticker evento;
tabla_score mi_score;

//--------------------------------------------------------------Configuración de red y contraseña


const char *ssid = "ESP8266 Access Point"; // The name of the Wi-Fi network that will be created
const char *password = "12345678";   // The password required to connect to it, leave blank for

/* lo pongo en cambiazo
  const char *red = "cecilia";
  const char *contra = "murcielago";
*/

//Objeto "servidor" en el puerto 80(puerto HTTP)
ESP8266WebServer server(80);


//char Mapa[32][26];

int u = 0;
bool banderita = 0;




// ----------------------------------------------------------------------------una pagina inical donde estan los formularios para cambiar el wifi.
void tomadatos() {
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
      padding-bottom: 35.2%;\
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
      min-width: 193px;\
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
        <p>&nbsp;&nbsp; Welcome to: <i><b><u>Snake</u> <u>the</u> <u>game</u></b></i>\
        </p>\
        <p>&nbsp;&nbsp; Good luck have fun! </p>\
        <div id='Contenedor2'>\
          <div id='Expl1'>&nbsp;Por favor ingresar una red WiFi y su contraseña</div>\
          <div id='Form'>\
            <form action='/' method='post'> <label for='ssid'>Nombre de red:</label><br>\
              <input id='ssid' name='ssid' type='text' autocapitalize='none'><br>\                      
              <br>\
              <form action='/' method='post'> <label for='contrasenia'>Contraseña:</label><br>\
              <input id='contrasenia' name='contrasenia' type='password'><br>\    
              <br>\
              <input value='Aceptar' type='submit'> </form>\
          </div>\
        </div>\
      </div>\
    </div>\
  </body>\
</html>";

  String a;
  a = (server.arg("ssid").c_str());
  //Serial.println(a);
  //Serial.println("ya imprimio ssid");

  String b;
  b = (server.arg("contrasenia").c_str());
  //Serial.println(b);
  //Serial.println("ya imprimio contraseña");

  if (a != "" && b != "")
    cambiazo(a, b);

  server.send(200, "text/html", pamandartomadatos);
  Serial.println("termino pagina toma de datos");



}

//----------------------------------------------------------------------------------------cuando entra a la pagina /aver, cambia de punto de acceso a cliente
void cambiazo( String red, String contra) {

  /*
    const char *red = "cecilia";
    const char *contra = "murcielago";
  */


  Serial.print("Conectando a ");
  Serial.println(red);

  WiFi.begin(red, contra);
  int a = 0;
  while (WiFi.status() != WL_CONNECTED) //espera una conexión
  {
    if ( a == 0) {
      Serial.print("Estableciendo conexión con... ");
      Serial.println(WiFi.SSID()); //Imprime el nombre de la red
      a = 1;
    }

    delay(599);
  }
  Serial.print("Conectado a red! direccion IP ESP -> ");
  Serial.print(WiFi.localIP()); //Imprime la IP local de ESP
  Serial.println("/Home");

  WiFi.mode(WIFI_STA);

  Serial.println("termino cambiazo");

}
//----------------------------------------------------------------------------------------inicializa serpiente
bool inicializar = 0;
void inicio() {
  generar_manzanas.attach_ms(8523 , generacion_manzana);
  velocidad_serpiente.attach_ms(390 , bandera);
  evento.attach_ms(100 , serialEvent);
  CDF = 0;

  inicializar = 1;
  //poner para que se redireccione a /Game
  Serial.println("termino inicio serpiente");
}


void estado() {
  server.send(200, "text/plain", "Hola Mundoo");

  Serial.println(WiFi.status());
}




//----------------------------------------------------------------------------------------htmlHome
String htmlHome() {
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
      padding-bottom: 35.2%;\
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
      min-width: 193px;\
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
        <p>&nbsp;&nbsp; Welcome to: <i><b><u>Snake</u> <u>the</u> <u>game</u></b></i>\
        </p>\
        <p>&nbsp;&nbsp; Good luck have fun! </p>\
        <div id='Contenedor2'>\
          <div id='Expl1'>&nbsp;Ingresa tu nombre y luego presiona en el botón para\
            comenzar el juego:</div>\
          <div id='Form'>\
            <form action='/Home' method='post'> <label for='name'>Name:</label><br>\
              <input id='name' name='name' maxlength='8' type='text'><br>\
              <br>\
              <input value='Jugar' type='submit'> </form>\
          </div>\
        </div>\
        <div id='Scoreboard'>\
          <table style='width:100%;margin-bottom: 18px;'>\
            <caption>Puntajes</caption>\
            <tbody>\
              <tr>\
                <th>Name</th>\
                <th>Score</th>\
              </tr>\
              <tr>\
                <td>HOLAHOLA\
                </td>\
                <td>220000\
                </td>\
              </tr>\
              <tr>\
                <td><br>\
                </td>\
                <td><br>\
                </td>\
              </tr>\
              <tr>\
                <td><br>\
                </td>\
                <td><br>\
                </td>\
              </tr>\
              <tr>\
                <td><br>\
                </td>\
                <td><br>\
                </td>\
              </tr>\
              <tr>\
                <td><br>\
                </td>\
                <td><br>\
                </td>\
              </tr>\
              <tr>\
                <td><br>\
                </td>\
                <td><br>\
                </td>\
              </tr>\
              <tr>\
                <td><br>\
                </td>\
                <td><br>\
                </td>\
              </tr>\
              <tr>\
                <td><br>\
                </td>\
                <td><br>\
                </td>\
              </tr>\
              <tr>\
                <td><br>\
                </td>\
                <td><br>\
                </td>\
              </tr>\
              <tr>\
                <td><br>\
                </td>\
                <td><br>\
                </td>\
              </tr>\
            </tbody>\
          </table>\
        </div>\
        <div id='Contenedor3'>\
      <h3> <u>About</u> <u>us</u> </h3>\
      <div id='Expl2'>\
              &nbsp;Proyecto de una materia de 6to año del instituto politécnico\
              superior de rosario en el que hacemos cosas.\
              No se que poner pero jaja.com\
      </div>\
        </div>\
      </div>\
    </div>\
  </body>\
</html>";





  Serial.println("termino htmlHome");
  return (pamandarhome);
}

//----------------------------------------------------------------------------------------htmlGame
String htmlGame() {
  int i = 0, j = 0;

  Serial.println("");

  String pamandargame = "";
  pamandargame += "<html> <head><link rel='stylesheet' href='resource://content-accessible/plaintext.css'> <meta http-equiv='refresh' content='0.3'> </head> <body> <pre>";



  for (j = 0; j < y; j++) {
    for (i = 0; i < x; i++) {
      pamandargame += Mapa[i][j];
    }
    pamandargame += '\n';
  }


  //pamandargame += soyelmapa;   probar con string en vez de char
  //Serial.println(soyelmapa);

  pamandargame += "</pre></body></html>";
  Serial.println("termino htmlGame");
  return (pamandargame);
}


//----------------------------------------------------------------------------------------MANEJO HOME
void handleHome() {


  String a="";
  a = (server.arg("name").c_str());
  Serial.println(a);
  Serial.println("ya imprimio");

  if (server.args() > 0) {
    if (server.hasArg("name")) {
      a = (server.arg("name").c_str());
      Serial.println(a);
      Serial.println("entro al segundo if");
    }
    server.send(200, "text/html", htmlGame());
    Serial.println("entro al primer if");
  }

  server.send(200, "text/html", htmlHome());

  if (a != ""){
    inicio();
    //server.send(200, "text/html", htmlGame()); //en vez de que te redireccione a /Game, que te muestre el juego en home, pero tambien hay que cambiar en el cdf que envie a /Home en vez de /Game
    }

  Serial.println("termino handleHome");
}

//----------------------------------------------------------------------------------------MANEJO GAME
void handleGame() {

  //Rutina asociada a la direccion web "/Game"

  server.send(200, "text/html", htmlGame());
  Serial.println("termino handleGame");
}




//-----------------------------------------------------------------------------------------SETUP
void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("empieza setup");

  /* lo pongo en la funcion inicio
    generar_manzanas.attach_ms(8523 , generacion_manzana);
    velocidad_serpiente.attach_ms(390 , bandera);
    evento.attach_ms(100 , serialEvent);
    CDF = 0;
  */
  strcpy(mi_score.nombre, "pepe");
  mi_score.puntaje = 100; ///// codigo auxiliar

  //pamandar.reserve(2000); creo que no lo usamos porque ya no es global

  /* -----lo pongo en cambiazo
    Serial.print("Conectando a ");
    Serial.println(red);

    WiFi.begin(red, contra);
    int a = 0;
    while (WiFi.status() != WL_CONNECTED) //espera una conexión
    {
      if ( a == 0) {
        Serial.print("Estableciendo conexión con... ");
        Serial.println(WiFi.SSID()); //Imprime el nombre de la red
        a = 1;
      }

      delay(599);
    }
    Serial.print("Conectado a red! direccion IP ESP -> ");
    Serial.print(WiFi.localIP()); //Imprime la IP local de ESP
    Serial.println("/Home");

  */


  WiFi.softAPdisconnect(0); // desconectar el AP

  //Serial.println(WiFi.status());
  //Serial.println(WL_CONNECTED); vale 3
  if (WiFi.status() != WL_CONNECTED) { //si ya esta conectado, que no cree el access point al dope.
    // poner una funcion para cambiar de red en /home si fuera necesario. no puede ser la que ya está (cambiazo) porque tiene el while que pregunta si ya esata conectado a otra red

    // para la toma de datos, la wemos es un punto de acceso
    WiFi.softAP(ssid, password);             // Start the access point
    Serial.print("Access Point \"");
    Serial.print(ssid);
    Serial.println("\" started");

    Serial.print("IP address:\t");
    Serial.println(WiFi.softAPIP());         // Send the IP address of the ESP8266 to the computer
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado a red! direccion IP ESP -> ");
    Serial.print(WiFi.localIP()); //Imprime la IP local de ESP
    Serial.println("/Home");
  }

  //Crea una asociación entre la direccion web y las funciones que seran utilizadas
  server.on("/Home", handleHome);  //// Manejo el ingreso de datos del html
  server.on("/Game", handleGame);
  server.on("/", tomadatos);
  //server.on("/inicio", inicio);
  server.on("/estado", estado);

  server.begin(); //Inicia el servidor


  Serial.println("termino setup");
}






//--------------------------------------------------------------LOOP
void loop() {
  //Analiza las solicitaciones via web
  server.handleClient();

  //Serial.println(CDF);
  if (inicializar == 1) {
    switch (CDF) {
      case 0: {
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
          PrintMapa();
          CDF++;
          break;
        }
      //-------------------------------------------------------------funcionamiento normal
      case 4: {
          if (bandera_mov == 1) {
            bandera_mov = 0;
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
          //htmlGame(); no hace falta pues cuando se llama a la funcion handle, se llama a la html
          CDF++;
          break;
        }

      case 8: {
          handleGame();   //se manda la cadena a la pagina
          CDF++;
          break;
        }

      case 9: {

          //Serial.println(pamandar); //imprime la cadena tipo String de caracteres en el monitor serie
          //Serial.println("termino de imprimir");
          CDF = 4;
          break;
        }

      default:
        Serial.println ("default del switch");
    }
  }
}






void marco () {//--------------------------------------------------------------------------------------Marco---LLena la matriz Mapa_N con números.
  Serial.println("marco inicio");
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

void movimiento() {//----------------------------------------------------------------------------------Función principal del snake.---------------------------------------------------------
  //Serial.println(Direccion);

  Serial.println("Movimiento inicio");
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

  if (llegoacod == -2 || llegoacod >= 2) {//--------------El jugador choca y pierde.
    Serial.println("Perdiste");
    Perdiste = 1;
    CDF = 0;
    generar_manzanas.detach();
    generar_manzanas.attach_ms(8523 , generacion_manzana);
  }
  else {
    if (llegoacod == 0) {
      Score += 100;
    }
    movimiento_numeros(ayudaX, ayudaY, llegoacod);//------La partida sigue su curso.
  }
  return;
}

void pregunta_manzana() {//----------------------------------------------------------------------------Se fija si hay manzanas en el mapa.-----------------------------------------------------
  if (manzanas_en_mapa == 0) generacion_manzana();
}

void generacion_manzana() {//--------------------------------------------------------------------------Genera y ubica una manzana en el mapa.--------------------------------------------------
  Serial.println("Generando manzana");
  int posx_nueva_manzana = random(1, x - 1);
  int posy_nueva_manzana = random(1, y - 1);
  if (Mapa_N[posx_nueva_manzana][posy_nueva_manzana] == -1) {
    Mapa_N[posx_nueva_manzana][posy_nueva_manzana] = 0;
    manzanas_en_mapa++;
  }
  else {
    generacion_manzana();
  }
  Serial.println("Manzana generada");
}

void nuevo_juego() {//---------------------------------------------------------------------------------Al perder el juego se reinician algunos valores.----------------------------------------
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

  /*
    //longitud = 11;
    Mapa_N[posx_Cola][posy_Cola + 1] = 4;
    Mapa_N[posx_Cola][posy_Cola + 2] = 5;
    Mapa_N[posx_Cola][posy_Cola + 3] = 6;
    Mapa_N[posx_Cola + 1][posy_Cola + 3] = 7;
    Mapa_N[posx_Cola + 2][posy_Cola + 3] = 8;
    Mapa_N[posx_Cola + 3][posy_Cola + 3] = 9;
    Mapa_N[posx_Cola + 4][posy_Cola + 3] = 10;
    Mapa_N[posx_Cola + 5][posy_Cola + 3] = 11;
  */
}

void movimiento_numeros(int ayudaX, int ayudaY, int llegoacod) {//-------------------------------------Lee la matriz Mapa_N y realiza el movimiento de la serpiente.---------------------------
  int i = 0;
  Serial.println("Moviendo numeros");
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
    case -1: {//------------------------------------------En caso de que no coma una manzana la cola se mueve.
        Mapa_N[ayudaX][ayudaY] = -1;
        break;
      }
    case 0: {//-------------------------------------------En caso de que coma una manzana la longitud aumenta.
        manzanas_en_mapa--;
        longitud++;
        break;
      }
    default: {
        Perdiste = 1;
        break;
      }
  }
  pregunta_manzana();
  Serial.println("Movimiento completado");
}

void conversor_numero_a_caracter(int FT) {//-----------------------------------------------------------Utiliza los números de la matriz Mapa_N y los reemplaza con caracteres en la matriz Mapa.
  //soyelmapa = "";
  for (int i = FT ; i < x - FT  ; i++) {
    for (int n = FT ; n < y - FT  ; n++) {
      switch (Mapa_N[i][n]) {
        case -1: {//--------------------------------------En caso de (-1), se deja un espacio en Mapa[][].
            Mapa[i][n] = ' ';
            //soyelmapa += " ";
            break;
          }
        case 0: {//---------------------------------------En caso de (0), se ubica una "O", la cual simboliza una manzana.
            //Mapa[i][n] = '■';
            //soyelmapa += "■";
            Mapa[i][n] = 'O';
            break;
          }
        case 1: {//---------------------------------------En caso de (1), se ubica la cabeza en un sentido en específico.
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
          }//---------------------------------------------En estos siguientes casos, Mapa[][] escribe los bordes del mapa.
        case -3: {//--------------------------------------En estos siguientes 5 casos se ubica el marco del Mapa.
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
        default: {//--------------------------------------Finalmente se ubica el cuerpo de la Snake.
            Mapa[i][n] = '*';
            //soyelmapa += "*";
            break;
          }
      }
    }
  }
}

void bandera() {//-------------------------------------------------------------------------------------Función que permite el funcionamiento correcto del CDF (Contador de Flujo)
  bandera_mov = 1;
}

void PrintMapa() {//-----------------------------------------------------------------------------------Muestreo del juego en el monitor serial.
  for (int z = 0; z < y ; z++) {
    for (int i = 0; i < x ; i++) {
      Serial.print(Mapa[i][z]);
      //Serial.print("/");
    }
    Serial.println("");
  }
}

void serialEvent() {
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
    if ((DireccionAnterior == 1 && Direccion == 2) || (DireccionAnterior == 2 && Direccion == 1) ) {
      Direccion = DireccionAnterior;
      Serial.println("Dirección inválida.");
    }
    else if ( (DireccionAnterior == 3 && Direccion == 4) || (DireccionAnterior == 4 && Direccion == 3) ) {
      Direccion = DireccionAnterior;
      Serial.println("Dirección inválida.");
    }
  }
}
