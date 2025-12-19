#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <U8g2lib.h>

/* ===================== CONFIGURACIÓN DE PINES ===================== */
// OLED I2C (Defecto ESP32: SDA=21, SCL=22)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 33
#define OLED_SCL 32
// DFPlayer Mini (UART2)
#define MP3_RX_PIN 25 // Conectar al TX del DFPlayer
#define MP3_TX_PIN 26 // Conectar al RX del DFPlayer (con resistencia 1k)

// Entradas
#define POT_PIN 34       // Potenciómetro para seleccionar ID (Pin solo entrada)
#define BTN_VOZ_PIN 4    // Botón 1: Descripción/Voz (001, 004, 007...)
#define BTN_GRITO_PIN 5  // Botón 2: Grito/Cry (002, 005, 008...)
#define BTN_MUSIC_PIN 18 // Botón 3: Música temática (003, 006, 009...)
#define BTN_STATS_PIN 19 // Botón 4: Alternar Vista (Sprite/Datos)

/* ===================== DATOS POKEMON ===================== */
// Estructura para almacenar la base de datos local
struct PokemonData {
  String nombre;
  String tipo;
  String altura;
  String peso;
  int ataque;
  int defensa;
  const unsigned char* sprite; // Puntero al bitmap del sprite
};

// Sprites de Pokémon (32x32 pixels)
const unsigned char PROGMEM sprite_bulbasaur[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 
0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x03, 0x56, 0x00, 0x00, 0x02, 0xdb, 0x00, 
0x00, 0x40, 0xbd, 0x00, 0x00, 0x78, 0x3c, 0x00, 0x00, 0x72, 0x00, 0x00, 0x00, 0xe3, 0x00, 0x00, 
0x00, 0xff, 0xc0, 0x00, 0x00, 0x7f, 0xa2, 0x00, 0x01, 0xe8, 0xfc, 0x00, 0x00, 0xf3, 0xec, 0x00, 
0x00, 0xf3, 0xc7, 0x00, 0x00, 0x3f, 0x46, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xe0, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM sprite_charmander[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 
	0x00, 0x1c, 0x01, 0x80, 0x00, 0x7e, 0x01, 0x80, 0x00, 0x7e, 0x03, 0xc0, 0x00, 0xff, 0x03, 0xc0, 
	0x00, 0xfb, 0x03, 0xc0, 0x01, 0xe3, 0x01, 0x80, 0x01, 0xf3, 0x01, 0x00, 0x00, 0xff, 0x81, 0x00, 
	0x00, 0x3f, 0x83, 0x00, 0x00, 0x0e, 0xc6, 0x00, 0x00, 0x0c, 0xee, 0x00, 0x00, 0x0d, 0xb4, 0x00, 
	0x00, 0x06, 0x70, 0x00, 0x00, 0x13, 0xe0, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0xa0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM sprite_togekiss[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x06, 0x80, 
	0x00, 0x0f, 0x06, 0x80, 0x00, 0x1e, 0xf9, 0x40, 0x06, 0xbe, 0x1c, 0x80, 0x0e, 0x78, 0xdf, 0x00, 
	0x06, 0xe5, 0x39, 0xc0, 0x01, 0x98, 0xb6, 0xc0, 0x01, 0xff, 0x77, 0xb8, 0x03, 0xfe, 0xf8, 0x7c, 
	0x03, 0xbf, 0xff, 0xfc, 0x03, 0xbf, 0xff, 0xf0, 0x01, 0xfc, 0x7f, 0x80, 0x00, 0xdf, 0x80, 0x00, 
	0x00, 0x3b, 0xd0, 0x00, 0x00, 0x06, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM sprite_pikachu[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x06, 0x00, 0xf0, 
	0x00, 0x06, 0x01, 0xf0, 0x00, 0x04, 0x33, 0xe0, 0x00, 0x1e, 0x77, 0xc0, 0x00, 0x3f, 0xef, 0x80, 
	0x00, 0x3f, 0xe7, 0x00, 0x00, 0x7e, 0xe3, 0x80, 0x00, 0x7c, 0xf1, 0x80, 0x00, 0x3f, 0xfb, 0x00, 
	0x00, 0x5f, 0xf0, 0x00, 0x00, 0x0f, 0x7c, 0x00, 0x00, 0x06, 0xd8, 0x00, 0x00, 0x07, 0x3c, 0x00, 
	0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM sprite_giratina[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x20, 0x00, 0x00, 0x03, 0x74, 0x00, 0x00, 0x03, 0x4c, 0x20, 0x00, 0x06, 0xb8, 0xc0, 0x00, 
	0x0b, 0xfb, 0xc0, 0x00, 0x0b, 0xe7, 0x80, 0x00, 0x09, 0xc7, 0x80, 0x00, 0x0d, 0x07, 0x80, 0x00, 
	0x06, 0x0e, 0x00, 0x00, 0x0b, 0xf9, 0x0c, 0x00, 0x09, 0xc3, 0x50, 0x00, 0x00, 0x07, 0x40, 0x00, 
	0x03, 0x0e, 0x81, 0x80, 0x0f, 0x01, 0x63, 0xe0, 0x02, 0x06, 0xfd, 0x80, 0x00, 0x40, 0x60, 0x00, 
	0x00, 0x0c, 0x1f, 0x00, 0x00, 0x01, 0xc8, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Base de datos de ejemplo (Puedes agregar más)
const int TOTAL_POKEMON = 5;
PokemonData pokedex[TOTAL_POKEMON] = {
  {"BULBASAUR",  "Planta/Veneno", "0.7m", "6.9kg", 49, 49, sprite_bulbasaur},
  {"CHARMANDER", "Fuego",         "0.6m", "8.5kg", 52, 43, sprite_charmander},
  {"TOGEKISS",   "Hada/Vuelo", "1.5m", "38kg", 50, 95, sprite_togekiss},
  {"PIKACHU", "Eléctrico",         "0.4m", "6.0kg", 55, 40, sprite_pikachu},
  {"GIRATINA", "Fantasma/Dragón",  "4.5m", "750kg",  100, 120, sprite_giratina}
};

/* ===================== OBJETOS GLOBALES ===================== */
DFRobotDFPlayerMini player;
// U8g2 para GME12864-43 (SH1106 con offset de 2 columnas)
U8G2_SH1106_128X64_VCOMH0_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA);
SemaphoreHandle_t xMutex; // Mutex para proteger recursos compartidos

/* ===================== ESTADO DEL SISTEMA ===================== */
int currentPokemonID = 0; // Índice del array (0 a TOTAL_POKEMON-1)
bool showStatsMode = false; // false = Vista Nombre, true = Vista Detalles
int volumenSistema = 25;  // Volumen seguro para altavoz 1W (0-30, max recomendado: 15)

// Tabla de mapeo corregida: [PokemonID][Boton] = Track
// Cada Pokémon tiene 3 audios: Descripción, Grito, Música
const int audioMap[TOTAL_POKEMON][3] = {
  {1,  2,  3},   // ID 0: Bulbasaur   -> BTN1=001(desc), BTN2=002(grito), BTN3=003(música)
  {4,  5,  6},   // ID 1: Charmander  -> BTN1=004(desc), BTN2=005(grito), BTN3=006(música)
  {7,  8,  9},   // ID 2: Togekiss    -> BTN1=007(desc), BTN2=008(grito), BTN3=009(música)
  {10, 11, 12},  // ID 3: Pikachu     -> BTN1=010(desc), BTN2=011(grito), BTN3=012(música)
  {13, 14, 15}   // ID 4: Giratina    -> BTN1=013(desc), BTN2=014(grito), BTN3=015(música)
};

// Bitmap simple de una Pokeball (16x16) para decorar
const unsigned char PROGMEM pokeball_icon[] = {
  0x07, 0xE0, 0x18, 0x18, 0x20, 0x04, 0x40, 0x02, 0x40, 0x02, 0x8F, 0xF1, 0x8F, 0xF1, 0x80, 0x01,
  0x80, 0x01, 0x40, 0x02, 0x40, 0x02, 0x20, 0x04, 0x18, 0x18, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00
};

/* ===================== FUNCIONES AUXILIARES ===================== */

// Dibuja la interfaz gráfica según el estado
void drawInterface() {
  u8g2.clearBuffer();
  
  if (!showStatsMode) {
    // === VISTA 1: SPRITE DEL POKÉMON ===
    // Título pequeño
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 8, "POKEDEX");
    
    // Icono Pokeball
    u8g2.drawXBM(110, 0, 16, 16, pokeball_icon);

    // DIBUJAR SPRITE (32x32) centrado
    int spriteX = (SCREEN_WIDTH - 32) / 2;  // Centrar horizontalmente
    int spriteY = 16;                        // Posición vertical
    u8g2.drawXBM(spriteX, spriteY, 32, 32, pokedex[currentPokemonID].sprite);

    // ID y Nombre debajo del sprite
    char buffer[20];
    sprintf(buffer, "#%03d %s", currentPokemonID + 1, pokedex[currentPokemonID].nombre.c_str());
    u8g2.drawStr(0, 60, buffer);
    
  } else {
    // === VISTA 2: ESTADÍSTICAS COMPLETAS ===
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Título
    char titulo[25];
    sprintf(titulo, "DATOS: %s", pokedex[currentPokemonID].nombre.c_str());
    u8g2.drawStr(0, 8, titulo);
    u8g2.drawHLine(0, 10, 128);

    // Tipo
    u8g2.drawStr(0, 20, "TIPO:");
    u8g2.drawStr(40, 20, pokedex[currentPokemonID].tipo.c_str());

    // Altura
    u8g2.drawStr(0, 30, "ALT.:");
    u8g2.drawStr(40, 30, pokedex[currentPokemonID].altura.c_str());

    // Peso
    u8g2.drawStr(0, 40, "PESO:");
    u8g2.drawStr(40, 40, pokedex[currentPokemonID].peso.c_str());

    // Ataque
    char atq[10];
    sprintf(atq, "%d", pokedex[currentPokemonID].ataque);
    u8g2.drawStr(0, 50, "ATQ:");
    u8g2.drawStr(40, 50, atq);

    // Defensa
    char def[10];
    sprintf(def, "%d", pokedex[currentPokemonID].defensa);
    u8g2.drawStr(0, 60, "DEF:");
    u8g2.drawStr(40, 60, def);
  }
  
  u8g2.sendBuffer();
}

/* ===================== TAREA 1: SENSORES Y PANTALLA ===================== */
// Se encarga de leer el potenciómetro y actualizar la OLED
void taskInterface(void *pvParameters) {
  int lastID = -1;
  bool lastMode = false;

  for (;;) {
    // 1. Leer Potenciómetro con suavizado
    int potValue = analogRead(POT_PIN);
    // Mapeamos 4095 (ESP32 ADC 12bit) al número de pokemones
    int leidoID = map(potValue, 0, 4095, 0, TOTAL_POKEMON - 1);
    leidoID = constrain(leidoID, 0, TOTAL_POKEMON - 1);

    // Debug: Mostrar lectura del potenciómetro
    Serial.print("POT: ");
    Serial.print(potValue);
    Serial.print(" -> ID: ");
    Serial.println(leidoID);

    // 2. Actualizar variable global protegida
    xSemaphoreTake(xMutex, portMAX_DELAY);
    if (leidoID != currentPokemonID) {
      currentPokemonID = leidoID;
      // Si cambiamos de pokemon, reseteamos a la vista principal
      showStatsMode = false; 
    }
    
    // Copias locales para verificar si hay que redibujar
    int idActual = currentPokemonID;
    bool modeActual = showStatsMode;
    xSemaphoreGive(xMutex);

    // 3. Redibujar solo si hubo cambios (Ahorra recursos I2C)
    if (idActual != lastID || modeActual != lastMode) {
      xSemaphoreTake(xMutex, portMAX_DELAY); // Protegemos el bus I2C
      drawInterface();
      xSemaphoreGive(xMutex);
      lastID = idActual;
      lastMode = modeActual;
    }

    vTaskDelay(pdMS_TO_TICKS(100)); // Revisar cada 100ms
  }
}

/* ===================== TAREA 2: BOTONES Y AUDIO ===================== */
// Maneja la lógica de los 4 botones y manda comandos al DFPlayer
void taskInputLogic(void *pvParameters) {
  // Estados anteriores para detectar flancos (Pullup: HIGH -> LOW)
  int lastVoz = HIGH;
  int lastGrito = HIGH;
  int lastMusic = HIGH;
  int lastStats = HIGH;

  for (;;) {
    // Leer estados actuales
    int btnVoz = digitalRead(BTN_VOZ_PIN);
    int btnGrito = digitalRead(BTN_GRITO_PIN);
    int btnMusic = digitalRead(BTN_MUSIC_PIN);
    int btnStats = digitalRead(BTN_STATS_PIN);

    // Obtener ID actual UNA SOLA VEZ al inicio del ciclo
    xSemaphoreTake(xMutex, portMAX_DELAY);
    int id = currentPokemonID;
    String nombre = pokedex[id].nombre; // Copiar también el nombre
    xSemaphoreGive(xMutex);

    // --- LÓGICA BOTÓN 1: DESCRIPCIÓN/VOZ ---
    if (lastVoz == HIGH && btnVoz == LOW) {
      int track = audioMap[id][0]; // Columna 0 = Botón 1 (Descripción)
      Serial.println("=====================================");
      Serial.print("[BTN 1 - DESCRIPCION] Pokemon ID: ");
      Serial.print(id);
      Serial.print(" | Nombre: ");
      Serial.print(nombre);
      Serial.print(" | Track solicitado: ");
      Serial.println(track);
      Serial.println("=====================================");
      player.play(track);
      vTaskDelay(pdMS_TO_TICKS(300)); // Debounce más largo
    }

    // --- LÓGICA BOTÓN 2: GRITO ---
    if (lastGrito == HIGH && btnGrito == LOW) {
      int track = audioMap[id][1]; // Columna 1 = Botón 2 (Grito)
      Serial.println("=====================================");
      Serial.print("[BTN 2 - GRITO] Pokemon ID: ");
      Serial.print(id);
      Serial.print(" | Nombre: ");
      Serial.print(nombre);
      Serial.print(" | Track solicitado: ");
      Serial.println(track);
      Serial.println("=====================================");
      player.play(track);
      vTaskDelay(pdMS_TO_TICKS(300));
    }

    // --- LÓGICA BOTÓN 3: MÚSICA ---
    if (lastMusic == HIGH && btnMusic == LOW) {
      int track = audioMap[id][2]; // Columna 2 = Botón 3 (Música)
      Serial.println("=====================================");
      Serial.print("[BTN 3 - MUSICA] Pokemon ID: ");
      Serial.print(id);
      Serial.print(" | Nombre: ");
      Serial.print(nombre);
      Serial.print(" | Track solicitado: ");
      Serial.println(track);
      Serial.println("=====================================");
      player.play(track);
      vTaskDelay(pdMS_TO_TICKS(300));
    }

    // --- LÓGICA BOTÓN 4: CAMBIAR VISTA ---
    if (lastStats == HIGH && btnStats == LOW) {
      xSemaphoreTake(xMutex, portMAX_DELAY);
      showStatsMode = !showStatsMode; // Alternar true/false
      Serial.print("[BTN 4] Vista cambiada a: ");
      Serial.println(showStatsMode ? "ESTADISTICAS" : "SPRITE");
      xSemaphoreGive(xMutex);
      vTaskDelay(pdMS_TO_TICKS(300));
    }

    // Actualizar estados anteriores
    lastVoz = btnVoz;
    lastGrito = btnGrito;
    lastMusic = btnMusic;
    lastStats = btnStats;

    vTaskDelay(pdMS_TO_TICKS(50)); // Ciclo rápido para respuesta ágil
  }
}

/* ===================== SETUP ===================== */
void setup() {
  // 1. Iniciar Serial Debug
  Serial.begin(115200);
  delay(1000); // Tiempo para que Serial se estabilice
  Serial.println("\n\n=== POKEDEX - INICIANDO ===");

  // 2. Configurar Pines
  Serial.println("Configurando pines...");
  pinMode(POT_PIN, INPUT); 
  // IMPORTANTE: INPUT_PULLUP para botones sin resistencias externas
  pinMode(BTN_VOZ_PIN, INPUT_PULLUP);
  pinMode(BTN_GRITO_PIN, INPUT_PULLUP);
  pinMode(BTN_MUSIC_PIN, INPUT_PULLUP);
  pinMode(BTN_STATS_PIN, INPUT_PULLUP);
  Serial.println("Pines configurados OK");

  // 3. Iniciar I2C con pines explícitos
  Serial.println("Iniciando I2C...");
  Wire.begin(OLED_SDA, OLED_SCL);  // SDA=21, SCL=22
  delay(100);
  Serial.println("I2C inicializado");

  // 4. Iniciar Pantalla OLED con U8g2
  Serial.println("Iniciando OLED SH1106 1.3\" con U8g2...");
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(15, 30, "POKEDEX V1.0");
  u8g2.drawStr(20, 45, "Cargando...");
  u8g2.sendBuffer();
  Serial.println("OLED inicializada correctamente!");
  delay(1500);

  // 5. Iniciar DFPlayer
  Serial.println("Iniciando DFPlayer Mini...");
  Serial2.begin(9600, SERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN);
  delay(1000); // DFPlayer necesita más tiempo para inicializarse
  
  if (!player.begin(Serial2)) {
    Serial.println(F("ERROR: DFPlayer no responde!"));
    Serial.println(F("Verifica: 1) Conexiones RX/TX, 2) Tarjeta SD, 3) Archivos MP3"));
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 15, "ERROR DFPlayer");
    u8g2.drawStr(0, 30, "Revisa SD/Audio");
    u8g2.sendBuffer();
    delay(3000);
    // Continuamos para permitir debug visual
  } else {
    Serial.println(F("DFPlayer Online!"));
    delay(500); // Esperar estabilización
    player.volume(volumenSistema); // Volumen inicial (0-30)
    Serial.print(F("Volumen configurado: "));
    Serial.println(volumenSistema);
    delay(200);
    
    // Debug: Mostrar cantidad de archivos en SD
    int fileCount = player.readFileCounts();
    Serial.print(F("Archivos en SD: "));
    Serial.println(fileCount);
  }

  // 6. Crear Mutex y Tareas FreeRTOS
  Serial.println("Creando tareas FreeRTOS...");
  xMutex = xSemaphoreCreateMutex();

  // Tarea Interfaz (Potenciómetro + OLED) - Núcleo 1
  xTaskCreatePinnedToCore(
    taskInterface,   // Función
    "Interface",     // Nombre
    4096,            // Stack size
    NULL,            // Parámetros
    1,               // Prioridad (Baja)
    NULL,            // Handle
    1                // Core
  );

  // Tarea Lógica (Botones + Audio) - Núcleo 1
  xTaskCreatePinnedToCore(
    taskInputLogic,  // Función
    "InputLogic",    // Nombre
    4096,            // Stack size
    NULL,            // Parámetros
    2,               // Prioridad (Alta - para que los botones respondan rápido)
    NULL,            // Handle
    1                // Core
  );
  
  Serial.println("=== SISTEMA LISTO ===");
  Serial.println("Usa el potenciometro para cambiar Pokemon");
  Serial.println("Botones: [1]VOZ | [2]GRITO | [3]MUSICA | [4]VISTA");
}

void loop() {
  // El loop se queda vacío porque FreeRTOS maneja todo en las tareas.
  vTaskDelay(portMAX_DELAY); 
}