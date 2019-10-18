#include <FastLED.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    21
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    98
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120

CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
    uint8_t brightness = 255;

    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void SegmentOff(int a, int b, int posision)
{
    a += posision * 49;
    b += posision * 49;
    for (int i = a; i <= b; i++)
    {
        leds[i].setRGB(0, 0, 0);
    }
}

void segments(char a, int i)
{
    Serial.println(a);
    switch (a)
    {
    case '0':
        SegmentOff(42, 48, i);
        if (i == 0)
            SegmentOff(0, 48, 0);
        break;
    case '1':
        SegmentOff(0, 13, i);
        SegmentOff(28, 48, i);
        break;
    case '2':
        SegmentOff(14, 20, i);
        SegmentOff(35, 41, i);
        break;
    case '3':
        SegmentOff(0, 6, i);
        SegmentOff(35, 41, i);
        break;
    case '4':
        SegmentOff(0, 13, i);
        SegmentOff(28, 34, i);
        break;
    case '5':
        SegmentOff(0, 6, i);
        SegmentOff(21, 27, i);
        break;
    case '6':
        SegmentOff(21, 27, i);
        break;
    case '7':
        SegmentOff(0, 13, i);
        SegmentOff(35, 48, i);
    case '8':
        break;
    case '9':
        SegmentOff(0, 6, i);
        break;
    case 'A':
        SegmentOff(7, 13, i);
        break;
    case 'B':
        break;
    case 'C':
        SegmentOff(14, 21, i);
        SegmentOff(35, 48, i);
        break;
    case 'D':
        SegmentOff(42, 48, i);
        break;
    case 'E':
        SegmentOff(14, 21, i);
        break;
    case 'F':
        SegmentOff(0, 21, i);
        break;
    case 'G':
        SegmentOff(21, 27, i);
        SegmentOff(46, 48, i);
        break;
    case 'H':
        SegmentOff(7, 13, i);
        SegmentOff(28, 34, i);
        break;
    }
}

void LEDinit()
{
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}