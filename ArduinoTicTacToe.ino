#define VRX_PIN A0 // Arduino pin connected to VRX pin
#define VRY_PIN A1 // Arduino pin connected to VRY pin
#define SW_PIN 13  // Arduino pin connected to SW pin

#include <Button.h>

// POTEZA
enum states
{
  FIRST,
  SECOND,
  OVER,
  NONE
};

// SMER PREMIKANJA
enum direction
{
  LEFT,
  RIGHT,
  UP,
  DOWN,
  STILL
};

// OS
enum axis
{
  X,
  Y
};

// PINi LED LUCK
int LEDs[][6] = {{0, 1, 2, 3, 4, 5}, {6, 7, 8, 9, 10, 11}, {12, 13, 14, 15, 16, 17}};

// PIN GUMBA NA JOYSTICKU
Button button(SW_PIN);

// STANJE MREZE 3x3
states arrayStatus[][3] = {{NONE, NONE, NONE}, {NONE, NONE, NONE}, {NONE, NONE, NONE}};

// POZICIJA
int i = 0;
int j = 0;

// INICIALIZACIJA POTEZE - ZACNE PRVI
states currentState = FIRST;

unsigned long previousMillis = 0; // Shrani zadnji preklop lucke
const long interval = 1000;       // Interval utripanja v ms

unsigned long joystickPreviousMillis = 0; // Shrani zadnje branje iz joysticka
const long joystickInterval = 100;        // Zakasnitev med naslednjim branjem joysticka v ms

// Funkcija najde naslednjo prosto LED diodo v mrezi
void moveToFreeSpot(enum direction currentDirection)
{
  // Ce ostanemo na mestu
  if (currentDirection == STILL)
    return;

  // Potrebno je ugasniti trenutno LED diodo preden se premaknemo
  if (arrayStatus[i][j] == NONE)
  {
    currentState == FIRST ? digitalWrite(LEDs[i][j], LOW) : digitalWrite(LEDs[i][j + 1], LOW);
  }

  // Se izvaja dokler ne najdemo proste LED diode
  while (true)
  {
    if (currentDirection == LEFT)
    {
      j--;
      if (j < 0)
      {
        i++;
        j = 2;
      }
    }
    else if (currentDirection == RIGHT)
    {
      j++;
      if (j > 2)
      {
        i++;
        j = 0;
      }
    }
    else if (currentDirection == DOWN)
    {
      i++;
      if (i > 2)
      {
        i = 0;
      }
    }
    else if (currentDirection == UP)
    {
      i--;
      if (i < 0)
      {
        i = 2;
      }
    }

    // Preveri ali je trenutna pozicija znotraj mreze in ali je LED dioda ugasnjena
    if (i >= 0 && i <= 2 && j >= 0 && j <= 2 && arrayStatus[i][j] == NONE)
    {
      break; // Izstopi, ce je, saj smo nasli iskano pozicijo.
    }
  }

  // Zacni z utripanjem trenutne LED diode
  currentState == FIRST ? digitalWrite(LEDs[i][j], HIGH) : digitalWrite(LEDs[i][j + 1], HIGH);
}

void setup()
{
  // Serial.begin(9600);

  // INICIALIZACIJA VSEH 18 DIGITALNIH PINOV
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      pinMode(LEDs[i][j], OUTPUT);
      digitalWrite(LEDs[i][j], LOW);
    }
  }

  button.begin();
}

void loop()
{
  // Preveri, ce je LED dioda prizgana ali ugasnjena enako ali dlje kot doloceni interval.
  // Ce je, jo preklopi glede na igralca na potezi
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    if (currentState == FIRST)
      digitalWrite(LEDs[i][j], !digitalRead(LEDs[i][j]));
    else
      digitalWrite(LEDs[i][j + 1], !digitalRead(LEDs[i][j + 1]));
  }

  // Preveri, ce je prekel interval za branje iz joysticka
  // Ce je, ga preberi
  unsigned long joystickCurrentMillis = millis();
  if (joystickCurrentMillis - joystickPreviousMillis >= joystickInterval)
  {
    joystickPreviousMillis = joystickCurrentMillis;
    readJoystick();
  }
}

// Interpretiraj prebrano vrednost iz joysticka
// Meje nastavimo na nedvoumno vrednost, da je interpretacija zanesljiva
enum direction interpretValue(int value, enum axis currentAxis)
{
  if (value > 800)
    return currentAxis == X ? RIGHT : DOWN;
  if (value < 200)
    return currentAxis == X ? LEFT : UP;
  return STILL;
}

// Preberi vrednost iz joysticka in preveri, ce je bila pritisnjena tipka
void readJoystick()
{
  // Preberi in interpretiraj prebrano vrednost
  enum direction xValue = interpretValue(analogRead(VRX_PIN), X);
  enum direction yValue = interpretValue(analogRead(VRY_PIN), Y);
  // Preberi gumb
  button.read();

  // Ce je gumb pritisnjen, prizgi LED diodo igralca na potezi
  // Preveri ali je igralec zmagal ali je mreza polna (neodlocen izzid)
  if (button.pressed())
  {
    currentState == FIRST ? digitalWrite(LEDs[i][j], HIGH) : digitalWrite(LEDs[i][j + 1], HIGH);
    arrayStatus[i][j] = currentState;

    if (checkWin(currentState))
    {
      reset();
    }
    else if (checkFull())
    {
      reset();
    }
    else
    {
      // Ce nihce ni zmagal, mreza pa ni polna, je na vrsti naslednji igralec
      currentState = (currentState == FIRST) ? SECOND : FIRST;
      // Pomaknemo se na zacetno LED diodo in najdemo naslednje prosto mesto
      i = 0;
      j = 0;
      moveToFreeSpot(RIGHT);
      return;
    }
  }

  // Premaknemo se na izbrano LED diodo
  moveToFreeSpot(xValue);
  moveToFreeSpot(yValue);
}

// Preveri, ce je mreza polna
// Pristop mogoce ni najboljsi, saj se izvaja v O(n^2), vendar je mreza dovolj majhna, da to ni problem
bool checkFull()
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      if (arrayStatus[i][j] == NONE)
      {
        return false;
      }
    }
  }
  return true;
}

// Ponastavi igro - ugasne vse LED diode in nastavi stanje na zacetno, prvi igralec na potezi
void reset()
{
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      arrayStatus[i][j] = NONE;
      digitalWrite(LEDs[i][j], LOW);
    }
  }
  i = 0;
  j = 0;
  currentState = FIRST;
}

// Preveri, ce je igralec zmagal
// Pristop je ponovno naiven
bool checkWin(states player)
{
  // Preveri vrstice in stolpce
  for (int i = 0; i < 3; i++)
  {
    if (arrayStatus[i][0] == player && arrayStatus[i][1] == player && arrayStatus[i][2] == player)
    {
      return true;
    }

    if (arrayStatus[0][i] == player && arrayStatus[1][i] == player && arrayStatus[2][i] == player)
    {
      return true;
    }
  }

  // Preveri diagonali
  if (arrayStatus[0][0] == player && arrayStatus[1][1] == player && arrayStatus[2][2] == player)
  {
    return true;
  }

  if (arrayStatus[0][2] == player && arrayStatus[1][1] == player && arrayStatus[2][0] == player)
  {
    return true;
  }

  // Ni zmagovalca
  return false;
}
