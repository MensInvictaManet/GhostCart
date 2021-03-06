#include <FastLED.h>

#define LED_DATA_PIN   			    2   			//  The WS2801 string data pin
#define LED_CLOCK_PIN			      4				//  The WS2801 string clock pin
#define POTENTIOMETER_PIN		    A0				//  The data pin to the potentiometer
#define BUTTON_PIN				      10				//  The pin that the button is connected to

#define NUM_LEDS        		    76  			//  The number of LEDs we want to alter
#define MAX_LEDS        		    76 	 		//  The number of LEDs on the full strip
#define NUM_LEDS_VIRTUAL		    MAX_LEDS + 10  	//  The number of LEDs to virtually travel. AKA the loop will continue until the position passes this number
#define BRIGHTNESS      		    60  			//  The number (0 to 200) for the brightness setting)

#define GHOST_CART				      true

int DELAY_TIME = 25;
unsigned long buttonTimer = 0;

//  Generic values
#define BUTTON_DELAY			      400

//  GlowFlow() values
#define GLOW_FLOW_DELAY         15
#define GLOW_FLOW_COLOR_TIME	  3000

//  MsPacman() values
#define RED_TO_WHITE_RATIO      6
#define MSPACMAN_SLOWDOWN       3

//  VegasMarquee() values
#define BLANK_TO_RED_RATIO      1
#define MARQUEE_SLOWDOWN        5

//  Fireworks() values
#define FIREWORK_DELAY			    500
#define FIREWORK_SIZE			      6

//  Fireballs() values
#define FIREBALL_COUNT          4
#define FIREBALL_DELAY          60
#define FIREBALL_FOLLOWERS      4
#define FIREBALL_COLOR_DELAY    5000

//  Cylon() values
#define CYLON_BAR_LENGTH        4
#define CYLON_BAR_DELAY         25
#define CYLON_COLOR_DELAY       5000
#define CYLON_MAX_POSITION      (NUM_LEDS - (CYLON_BAR_LENGTH - 1))
#define CYLON_MAX_DISTANCE      (NUM_LEDS - (CYLON_BAR_LENGTH - 1)) * 2

//  NewKITT() values
#define KITT_BAR_LENGTH         8
#define KITT_BAR_DELAY          25
#define KITT_COLOR_DELAY        5000
#define KITT_MAX_POSITION       (NUM_LEDS - KITT_BAR_LENGTH)

//  Sparkle() values
#define SPARKLE_DELAY           0
#define SPARKLE_COLOR_DELAY     5000

//  SnowSparkle() values
#define SNOWSPARKLE_DELAY           20
#define SNOWSPARKLE_POSTDELAY_MIN   100
#define SNOWSPARKLE_POSTDELAY_MAX   1000
#define SNOWSPARKLE_COLOR_DELAY     5000

//  TheatreChaseRainbow() values
#define THEATRECHASERAINBOW_DELAY   50


CRGB leds[MAX_LEDS];
int position = 0;

int LoopedLEDIndex(int index)
{
	index = index % MAX_LEDS;
	while (index < 0) index = (MAX_LEDS + index) % MAX_LEDS;
	return index;
}

struct Color
{
public:
	Color(int r, int g, int b) : 
		R(r),
		G(g),
		B(b)
	{}

	int R;
	int G;
	int B;
	
	inline bool isZero()		const {	return (R == 0 && G == 0 && B == 0);	}
};

enum LightGroupNames 
{
	LG_TOP,
	LG_BOTTOM,
	LG_LEFT,
	LG_RIGHT,
	LIGHT_GROUP_COUNT
};

struct LightGroup
{
public:
	LightGroup(int start, int end) : 
		Start(start),
		End(end)
	{}
	
	LightGroup(int groupIndex)
	{
		switch (groupIndex)
		{
		case LG_TOP:			Start = 0;		End = 23; 	break;
		case LG_RIGHT:			Start = 24;		End = 37;	break;
		case LG_BOTTOM:			Start = 38;		End = 61;	break;
		case LG_LEFT:			Start = 62;		End = 75;	break;
		}
	}
	
	int Start;
	int End;
};

void SetGroupColor(int group, CRGB color)
{
	LightGroup groupPair(group);
	for (int i = groupPair.Start; i <= groupPair.End; ++i) { leds[i] = color; }
	FastLED.show();
}

//  The basic colors to warp through for GlowFlow
#define COMMON_COLOR_COUNT 		15
const PROGMEM byte colors[COMMON_COLOR_COUNT][3] = 
{
	{ 0, 0, 0 },		//  Black
	{ 147, 112, 219 },	//  Medium Purple
	{ 199, 21, 133 },	//  Medium Violet Red
	{ 255, 20, 147 },	//  Deep Pink
	{ 255, 0, 0 },		//  Red
	{ 255, 140, 0 },	//  Dark Orange
	{ 255, 69, 0 },		//  Orange Red
	{ 255, 165, 0 },	//  Orange
	{ 200, 200, 0 },	//  Yellow
	{ 034, 139, 034 },	//  Forest Green
	{ 0, 250, 154 },	//  Medium Spring Green
	{ 032, 178, 170 },  //  Light Sea Green
	{ 0, 100, 0 },  	//  Dark Green
	{ 0, 255, 255 },  	//  Cyan
	{ 070, 130, 180 },  //  Steel Blue
};

//////////////////////////////
//  HELPER FUNCTIONS
//////////////////////////////
CRGB Wheel(byte wheelPosition)
{
    wheelPosition = 255 - wheelPosition;
    if(wheelPosition < 85)
    {
        return CRGB(255 - wheelPosition * 3, 0, wheelPosition * 3);
    }
    else if(wheelPosition < 170)
    {
        wheelPosition -= 85;
        return CRGB(0, wheelPosition * 3, 255 - wheelPosition * 3);
    }
    else
    {
        wheelPosition -= 170;
        return CRGB(wheelPosition * 3, 255 - wheelPosition * 3, 0);
    }
}

byte * Wheel2(byte WheelPos) {
  static byte c[3];
  
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }

  return c;
}

void ClearStrip()
{
	fill_solid(leds, MAX_LEDS, CRGB::Black);
}

void SetStrip(CRGB color)
{
  fill_solid(leds, MAX_LEDS, color);
}

inline bool IsPositionOnStrip(int position)
{
	return ((position >= 0) && (position < NUM_LEDS));
}

//////////////////////////////
//  PATTERN FUNCTIONS
//////////////////////////////
void Pacman()
{
  //  Clear the entire strip
  ClearStrip();
  
	static int pacmanPosition = 0;
  
	int P1 = ((pacmanPosition >= NUM_LEDS_VIRTUAL) ? 0 : pacmanPosition);
	int P2 = (((P1 - 3) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 3) : (P1 - 3));
	int P3 = (((P1 - 5) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 5) : (P1 - 5));
	int P4 = (((P1 - 7) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 7) : (P1 - 7));
	int P5 = (((P1 - 9) < 0) ? (NUM_LEDS_VIRTUAL + P1 - 9) : (P1 - 9));
	
	if (IsPositionOnStrip(P1)) leds[P1] = CRGB::Yellow;
	if (IsPositionOnStrip(P2)) leds[P2] = CRGB::Red;
	if (IsPositionOnStrip(P3)) leds[P3] = CRGB::Cyan;
	if (IsPositionOnStrip(P4)) leds[P4] = CRGB::Orange;
	if (IsPositionOnStrip(P5)) leds[P5] = CRGB::Magenta;
	
	FastLED.show();
	pacmanPosition = P1 + 1;
	delay(DELAY_TIME);
}

void RainbowFlow1()
{
	static int hue = 0;
	hue++;
	fill_rainbow(leds, MAX_LEDS, hue, -3);
	FastLED.show();
}

void RainbowFlow2(int speed = 1, bool berzerk = false)
{
	static int rainbowPosition = 0;
	for (int i = 0; i < MAX_LEDS; ++i)
    {
    	leds[i] = Wheel(((i * 256 / MAX_LEDS) + rainbowPosition) & 255);
    	if (berzerk) rainbowPosition++;
    }
    
    if (!berzerk) rainbowPosition += speed;
    delay(50);
	FastLED.show();
}

void Fire(int R, int G, int B)
{
	int r = R;
	int g = G;
	int b = B;

	for (int i = 0; i < MAX_LEDS; i++)
	{
		int flicker = random(0,150);
		int r1 = r - flicker;
		int g1 = g - flicker;
		int b1 = b - flicker;
		if (g1 < 0) g1 = 0;
		if (r1 < 0) r1 = 0;
		if (b1 < 0) b1 = 0;
		leds[i] = CRGB(r1, g1, b1);
	}
	FastLED.show();
	delay(random(50,150));
}

void GlowFlow()
{
	static Color colorCurrent(0, 0, 0);
	static Color colorDelta(0, 0, 0);
	static bool colorDisplay = true;
	static unsigned long nextChangeTime = 0;
	static bool adding = false;
	
	adding = false;
	if (colorDelta.R > 0) { colorDelta.R -= 1; colorCurrent.R = min(255, colorCurrent.R + 1); delay(GLOW_FLOW_DELAY); adding = true; }
	if (colorDelta.G > 0) { colorDelta.G -= 1; colorCurrent.G = min(255, colorCurrent.G + 1); delay(GLOW_FLOW_DELAY); adding = true; }
	if (colorDelta.B > 0) { colorDelta.B -= 1; colorCurrent.B = min(255, colorCurrent.B + 1); delay(GLOW_FLOW_DELAY); adding = true; }
	if (adding == false)
	{
		if (colorDelta.R < 0) { colorDelta.R += 1; colorCurrent.R = max(0,   colorCurrent.R - 1); delay(GLOW_FLOW_DELAY); }
		if (colorDelta.G < 0) { colorDelta.G += 1; colorCurrent.G = max(0,   colorCurrent.G - 1); delay(GLOW_FLOW_DELAY); }
		if (colorDelta.B < 0) { colorDelta.B += 1; colorCurrent.B = max(0,   colorCurrent.B - 1); delay(GLOW_FLOW_DELAY); }
	}
	
	CRGB currentCRGB(colorCurrent.R, colorCurrent.G, colorCurrent.B);
	fill_solid(leds, MAX_LEDS, currentCRGB);
	FastLED.show();
		
	if (colorDelta.isZero() && nextChangeTime < millis())
	{
		int newColorIndex = 0;
		do
		{
			newColorIndex = 1 + random(COMMON_COLOR_COUNT - 1);
			
			colorDelta.R = int(pgm_read_byte_near(colors[newColorIndex] + 0)) - colorCurrent.R;
			colorDelta.G = int(pgm_read_byte_near(colors[newColorIndex] + 1)) - colorCurrent.G;
			colorDelta.B = int(pgm_read_byte_near(colors[newColorIndex] + 2)) - colorCurrent.B;
			
		} while (colorDelta.isZero());
		
		nextChangeTime = millis() + GLOW_FLOW_COLOR_TIME;
	}
}

void GroupTest()
{
  //  Clear the entire strip
  ClearStrip();
  
	LightGroupNames group = LightGroupNames((millis() / 250) % LIGHT_GROUP_COUNT);
	
	SetGroupColor(group, CRGB::Red);
	FastLED.show();
}

void MsPacmanRedWhite()
{
  //  Clear the entire strip
  ClearStrip();
  
	static int whitePosition = 0;
	
	for (int i = 0; i < MAX_LEDS; i++)
	{
		leds[i] = ((((whitePosition / MSPACMAN_SLOWDOWN) + i) % (RED_TO_WHITE_RATIO + 1)) == 0) ? CRGB::White : CRGB::Red;
	}
	
	FastLED.show();
	whitePosition += 1;
	if (whitePosition == ((RED_TO_WHITE_RATIO + 1) * MSPACMAN_SLOWDOWN)) whitePosition = 0;
	
	delay(DELAY_TIME);
}

void VegasMarquee(int r, int g, int b)
{
  CRGB mainColor(r, g, b);
  
  //  Clear the entire strip
  ClearStrip();
  
	static int redPosition = 0;
	
	for (int i = 0; i < MAX_LEDS; i++)
	{
		leds[i] = ((((redPosition / MARQUEE_SLOWDOWN) + i) % (BLANK_TO_RED_RATIO + 1)) == 0) ? mainColor : CRGB::Black;
	}
	
	FastLED.show();
	redPosition += 1;
	if (redPosition == ((BLANK_TO_RED_RATIO + 1) * MARQUEE_SLOWDOWN)) redPosition = 0;
	
	delay(DELAY_TIME);
}

void FireworksSetup()
{
	for (int i = 0; i < MAX_LEDS; ++i)
	{
		int colorIndex = 1 + random(COMMON_COLOR_COUNT - 1);
		leds[i] = CRGB(int(pgm_read_byte_near(colors[colorIndex] + 0)), int(pgm_read_byte_near(colors[colorIndex] + 1)), int(pgm_read_byte_near(colors[colorIndex] + 2)));
	}
	FastLED.show();
}

void Fireworks()
{
	static unsigned long lastFireworkTime = 0;

	unsigned long currentTime = millis();
	
	//  Create a new firework if enough time has passed
	if (currentTime > lastFireworkTime + FIREWORK_DELAY)
	{
		int position = random(MAX_LEDS);
		int colorIndex = 1 + random(COMMON_COLOR_COUNT - 1);
		for (int i = 0; i < FIREWORK_SIZE; ++i) leds[LoopedLEDIndex(position + i)] = CRGB(int(pgm_read_byte_near(colors[colorIndex] + 0)), int(pgm_read_byte_near(colors[colorIndex] + 1)), int(pgm_read_byte_near(colors[colorIndex] + 2)));
		lastFireworkTime = currentTime;
	}
	
	//  Render the fireworks
	FastLED.show();
}

void Fireballs(int r1, int g1, int b1, int r2, int g2, int b2)
{
  //  Clear the entire strip
  ClearStrip();

  CRGB mainColor(r1, g1, b1);
  CRGB followColor(r2, g2, b2);
  
  unsigned long currentTime = millis();
  int iteration = (currentTime / FIREBALL_DELAY) % NUM_LEDS;
  
  for (int i = 0; i < FIREBALL_COUNT; ++i)
  {
    leds[LoopedLEDIndex(iteration + (i * NUM_LEDS / FIREBALL_COUNT))] = mainColor;
    for (int j = 0; j < FIREBALL_FOLLOWERS; ++j)
    {
      leds[LoopedLEDIndex(iteration + (i * NUM_LEDS / FIREBALL_COUNT) - 1 - j)] = followColor;
    }
  }
 
  //  Render the fireballs
  FastLED.show();
}

void CylonColor(CRGB& ledRef)
{
  switch((millis() / CYLON_COLOR_DELAY) % 4)
  {
    case 0:
      ledRef = CRGB::Red;
      break;
    case 1:
      ledRef = CRGB::Green;
      break;
    case 2:
      ledRef = CRGB::Blue;
      break;
    case 3:
      ledRef = CRGB::White;
      break;
  }
}

void Cylon()
{
  //  Clear the entire strip
  ClearStrip();

  int barPosition = (millis() / CYLON_BAR_DELAY) % CYLON_MAX_DISTANCE;
  if (barPosition > CYLON_MAX_POSITION) barPosition = CYLON_MAX_POSITION - (barPosition - CYLON_MAX_POSITION);

  for (int i = 0; i < CYLON_BAR_LENGTH; ++i)
  {
    CylonColor(leds[barPosition + i]);
  }
  
  //  Render the cylon bar
  FastLED.show();
}

void NewKITT(int r1, int g1, int b1, int r2, int g2, int b2)
{
  CRGB mainColor(r1, g1, b1);
  CRGB edgeColor(r2, g2, b2);
  
  //  Clear the entire strip
  ClearStrip();

  int barPosition = (millis() / KITT_BAR_DELAY) % KITT_MAX_POSITION;
  int patternIndex = (millis() / (KITT_BAR_DELAY * KITT_MAX_POSITION)) % 4;
  
  Serial.print(patternIndex);
  Serial.print(" - ");
  Serial.println(barPosition);
  
  switch (patternIndex)
  {
    case 0: //  first to last bar
      for (int i = 0; i < KITT_BAR_LENGTH; ++i)
      {
        leds[barPosition + i] = ((i == 0 || i == KITT_BAR_LENGTH - 1) ? edgeColor : mainColor);
      }
      break;
    case 1:
    case 3:
      for (int i = 0; i < KITT_BAR_LENGTH; ++i)
      {
        leds[((barPosition <= (KITT_MAX_POSITION / 2)) ? barPosition : (KITT_MAX_POSITION - barPosition)) + i] = ((i == 0 || i == KITT_BAR_LENGTH - 1) ? edgeColor : mainColor);
        leds[((((KITT_MAX_POSITION - barPosition) > (KITT_MAX_POSITION / 2)) ? (KITT_MAX_POSITION - barPosition) : barPosition) + i)] = ((i == 0 || i == KITT_BAR_LENGTH - 1) ? edgeColor : mainColor);
      }
      break;
    case 2: //  first to last bar
      for (int i = 0; i < KITT_BAR_LENGTH; ++i)
      {
        leds[KITT_MAX_POSITION - barPosition + i] = ((i == 0 || i == KITT_BAR_LENGTH - 1) ? edgeColor : mainColor);
      }
      break;
  }
  
  //  Render the display
  FastLED.show();
}

void SparkleColor(CRGB& ledRef)
{
  switch((millis() / SPARKLE_COLOR_DELAY) % 4)
  {
    case 0:
      ledRef = CRGB::Red;
      break;
    case 1:
      ledRef = CRGB::Green;
      break;
    case 2:
      ledRef = CRGB::Blue;
      break;
    case 3:
      ledRef = CRGB::White;
      break;
  }
}

void Sparkle(int r, int g, int b)
{
  CRGB sparkleColor(r, g, b);
  int randomLED = random(NUM_LEDS);
  leds[randomLED] = sparkleColor;
   
  //  Render the display
  FastLED.show();
  
  delay(SPARKLE_DELAY);
  leds[randomLED] = CRGB::Black;
}

void SnowSparkle(int r1, int g1, int b1, int r2, int g2, int b2)
{
  CRGB mainColor(r1, g1, b1);
  CRGB sparkleColor(r2, g2, b2);
  
  static int randomLED = 0;
  static int delayTime = 0;
  static long int nextSparkle = 0;

  if (millis() < nextSparkle) return;
  
  leds[randomLED] = mainColor;
  
  //  Render the display
  FastLED.show();
  
  delay(delayTime);
  
  SetStrip(mainColor);
  
  randomLED = random(NUM_LEDS);
  delayTime = random(SNOWSPARKLE_POSTDELAY_MIN, SNOWSPARKLE_POSTDELAY_MAX);
  
  leds[randomLED] = sparkleColor;
   
  //  Render the display
  FastLED.show();

  nextSparkle = millis() + SNOWSPARKLE_DELAY;
}

void TheatreChaseRainbow()
{
  byte *c;
  
  static int j = 0;
  static int q = 0;
  static int i = 0;
  
  for (int i=0; i < NUM_LEDS; i=i+3) {
    if (i + q >= MAX_LEDS) continue;
    c = Wheel2( (i+j) % 255);
    leds[i+q] = CRGB(*c, *(c+1), *(c+2));    //turn every third pixel on
  }
  
  //  Render the display
  FastLED.show();
 
  delay(THEATRECHASERAINBOW_DELAY);
 
  for (int i=0; i < NUM_LEDS; i=i+3) {
    if (i + q >= MAX_LEDS) continue;
    leds[i+q] = CRGB::Black;        //turn every third pixel off
  }

  if (++q >= 3)
  {
    q = 0;
    if (++j >= 256) j = 0;
  }
}


void setup()
{
	pinMode(POTENTIOMETER_PIN, INPUT);
	pinMode(BUTTON_PIN, INPUT_PULLUP); // connect internal pull-up
	
	//  Setup the LED strip and color all LEDs black
	if (GHOST_CART)	FastLED.addLeds<WS2801, LED_DATA_PIN, LED_CLOCK_PIN, RGB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
	else 			FastLED.addLeds<WS2811, LED_DATA_PIN, GRB>(leds, MAX_LEDS).setCorrection( TypicalLEDStrip );
	FastLED.setBrightness(BRIGHTNESS);
	ClearStrip();
	FastLED.show();
	
	//  Seed the random number generator
	randomSeed(analogRead(0));
	
	//  Set the button timer to the current time
	buttonTimer = millis();

  Serial.begin(9600);
  while (!Serial) { ; }
  Serial.println("Program START!");
}

void loop()
{
	static long int PatternCount = 17;
	static long int patternIndex = 0;
	
	//  Note: Comment this in ONLY if you have a potentiometer attached to POTENTIOMETER_PIN
	//DELAY_TIME = analogRead(POTENTIOMETER_PIN);
	//DELAY_TIME = map(DELAY_TIME, 0, 1023, 5, 45);
	
	unsigned long currentMillis = millis();
	if (buttonTimer < currentMillis)
	{
		if (digitalRead(BUTTON_PIN) == LOW)
		{
			buttonTimer = currentMillis + BUTTON_DELAY;
			++patternIndex;
			ClearStrip();
			if (patternIndex == 10) FireworksSetup();
		}
	}
	
	switch (patternIndex)
	{
		case 0:		RainbowFlow1();				                    break;
		case 1:		RainbowFlow2();				                    break;
		case 2:		RainbowFlow2(0, true);	                  break;
		case 3:		Pacman();					                        break;
    case 4:   Fire(255, 0, 0);                          break;
    case 5:   Fire(0, 255, 0);                          break;
    case 6:   Fire(0, 0, 255);                          break;
		case 7:		Fire(255, 175, 40);			                  break;
    case 8:   Fire(255, 255, 0);                        break;
    case 9:   Fire(128, 0, 128);                        break;
    case 10:  Fire(255, 40, 255);                       break;
    case 11:  Fire(255, 255, 255);                      break;
		case 12:  GlowFlow();					                      break;
		case 13:	GroupTest();				                      break;
		case 14:	MsPacmanRedWhite();	                      break;
		case 15:	VegasMarquee(255, 0, 0);			            break;
    case 16:  VegasMarquee(0, 255, 0);                  break;
    case 17:  VegasMarquee(0, 0, 255);                  break;
    case 18:  VegasMarquee(255, 175, 40);               break;
    case 19:  VegasMarquee(255, 255, 0);                break;
    case 20:  VegasMarquee(128, 0, 128);                break;
    case 21:  VegasMarquee(255, 40, 255);               break;
    case 22:  VegasMarquee(255, 255, 255);              break;
		case 23:  Fireworks();				                      break;
    case 24:  Fireballs(255, 0, 0, 25, 0, 0);           break;
    case 25:  Fireballs(0, 255, 0, 0, 25, 0);           break;
    case 26:  Fireballs(0, 0, 255, 0, 0, 25);           break;
    case 27:  Fireballs(255, 175, 40, 25, 17, 4);       break;
    case 28:  Fireballs(255, 255, 0, 25, 25, 0);        break;
    case 29:  Fireballs(128, 0, 128, 12, 0, 12);        break;
    case 30:  Fireballs(255, 40, 255, 25, 4, 25);       break;
    case 31:  Fireballs(255, 255, 255, 25, 25, 25);     break;
    case 32:  Cylon();                                  break;
    case 33:  NewKITT(255, 0, 0, 25, 0, 0);             break;
    case 34:  NewKITT(0, 255, 0, 0, 25, 0);             break;
    case 35:  NewKITT(0, 0, 255, 0, 0, 25);             break;
    case 36:  NewKITT(255, 175, 40, 25, 17, 4);         break;
    case 37:  NewKITT(255, 255, 0, 25, 25, 0);          break;
    case 38:  NewKITT(128, 0, 128, 12, 0, 12);          break;
    case 39:  NewKITT(255, 40, 255, 25, 4, 25);         break;
    case 40:  NewKITT(255, 255, 255, 25, 25, 25);       break;
    case 41:  Sparkle(255, 0, 0);                       break;
    case 42:  Sparkle(0, 255, 0);                       break;
    case 43:  Sparkle(0, 0, 255);                       break;
    case 44:  Sparkle(255, 175, 40);                    break;
    case 45:  Sparkle(255, 255, 0);                     break;
    case 46:  Sparkle(128, 0, 128);                     break;
    case 47:  Sparkle(255, 40, 255);                    break;
    case 48:  Sparkle(255, 255, 255);                   break;
    case 49:  SnowSparkle(255, 0, 0, 25, 0, 0);         break;
    case 50:  SnowSparkle(0, 255, 0, 0, 25, 0);         break;
    case 51:  SnowSparkle(0, 0, 255, 0, 0, 25);         break;
    case 52:  SnowSparkle(255, 175, 40, 25, 17, 4);     break;
    case 53:  SnowSparkle(255, 255, 0, 25, 25, 0);      break;
    case 54:  SnowSparkle(128, 0, 128, 12, 0, 12);      break;
    case 55:  SnowSparkle(255, 40, 255, 25, 4, 25);     break;
    case 56:  SnowSparkle(255, 255, 255, 25, 25, 25);   break;
    case 57:  TheatreChaseRainbow();                    break;
    default:  patternIndex = 0;                         break;
	}
}
