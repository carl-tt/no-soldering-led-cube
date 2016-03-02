// Arduino WS2811 Digital RGB LED Cube
// 2016 Carl Sutter
//
// Pin 6 is connected to the data line of the 125 WS2811 LED strand

#include <Adafruit_NeoPixel.h>
#include <string.h>

#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 3

#define PIN 6    // the i/o pin the data line on the WS2812b string is connected to
#define WIDTH 5  // the number of pixels in each dimension width=height=length

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(125, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


void loop()  //ALL VISUAL EFFECTS ARE STARTED FROM HERE
{
  if (true)
  //if (false)
  {

    effect_planboing(AXIS_Z, 60);
    effect_planboing(AXIS_Y, 60);
    effect_planboing(AXIS_X, 60);

    effect_blinky2();
    effect_blinky2();

    effect_random_filler(150,1);
    effect_random_filler(150,0);

    effect_rain(100);

    effectBoxZoom(100);

    for(int count=0; count<2; count++){
      effect_boxside_randsend_parallel(AXIS_X, 0, 25);
      effect_boxside_randsend_parallel(AXIS_X, 1, 25);
    }
    for(int count2=0; count2<2; count2++){
      effect_boxside_randsend_parallel(AXIS_Y, 0, 25);
      effect_boxside_randsend_parallel(AXIS_Y, 1, 25);
    }
    for(int count3=0; count3<2; count3++){
      effect_boxside_randsend_parallel(AXIS_Z, 0, 25);
      effect_boxside_randsend_parallel(AXIS_Z, 1, 25);
    }
  } // if true
  
} // loop


// ==========================================================================================
//   Effect functions
// ==========================================================================================

// Draw a plane on one axis and send it back and forth once.
void effect_planboing (int plane, int wait)
{
  int i;
  for (i=0;i<WIDTH;i++)
  {
    colorFill(strip.Color(0, 0, 0)); // Black
    setplane(plane, i);
    strip.show();
    delay(wait);
  }

  for (i=(WIDTH - 1);i>=0;i--)
  {
    colorFill(strip.Color(0, 0, 0)); // Black
    setplane(plane,i);
    strip.show();
    delay(wait);
  }
} // effect_planboing


// effect_blinky2 - flash the whole cube on and off at different speeds
void effect_blinky2()
{
  int i;
  
  i = 750;
  while (i>0) {
    colorWipe(strip.Color(0, 0, 0)); // Black
    delay_ms(i);

    colorWipe(strip.Color(0, 0, 255)); // Blue
    delay(10);

    i = i - (15+(1000/(i/10)));
  }

  delay(200);

  i = 750;
  while (i>0) {
    colorWipe(strip.Color(0, 0, 0)); // Black
    delay_ms(751-i);

    colorWipe(strip.Color(0, 0, 255)); // Blue
    delay(10);

    i = i - (15+(1000/(i/10)));
    }
} // effect_blinky2


// Set or clear exactly WIDTH^3 voxels in a random order.
void effect_random_filler(int wait, int state) {
  int x,y,z;
  int loop = 0;

  if (state == 1) {
    colorWipe(strip.Color(0, 0, 0)); // Black
  } else {
    colorWipe(strip.Color(0, 0, 255)); // Blue
  }

  while (loop<(WIDTH*WIDTH*WIDTH-1)) {
    x = rand()%WIDTH;
    y = rand()%WIDTH;
    z = rand()%WIDTH;

    if (state == 0 && getPixelColorAt(x, y, z) == 0x0000FF) clrVoxel(x, y, z);
    if (state == 1 && getPixelColorAt(x, y, z) == 0x000000) setVoxel(x, y, z);
    strip.show();
    delay_ms(wait);
    loop++;
  }
} // effect_random_filler


// effect-rain - do a rain falling show
void effect_rain(int iterations) {
  int i, j;
  int rnd_x;
  int rnd_y;
  int rnd_num;

  colorWipe(strip.Color(0, 0, 0)); // Black

  for (j=0; j<iterations; j++) {
    rnd_num = rand() % 4;

    for (i=0; i < rnd_num;i++) {
      rnd_x = rand()%WIDTH;
      rnd_y = rand()%WIDTH;
      setVoxel(rnd_x, rnd_y, (WIDTH - 1));
      strip.show();
    }

    delay(30);
    shift_z_down();
    clrplane_z(WIDTH-1);
  } // for j
} // effect_rain


// effectBoxZoom - draw an expanding then contracting box
void effectBoxZoom(int wait) {
  int i;
  
  colorFill (strip.Color(0, 0, 0)); // Black
  setVoxel(0, 0, 0);
  strip.show();
  delay(wait);    
  
  for (i=0; i<WIDTH; i++) {
    box_filled(0, 0, 0, i, i, i);
    delay(wait);    
  }

  for (i=1; i<WIDTH-1; i++) {
    colorWipe(strip.Color(0, 0, 0)); // Black
    box_filled(i, i, i, WIDTH-1, WIDTH-1, WIDTH-1);
    delay(wait);    
  }

  colorWipe(strip.Color(0, 0, 0)); // Black
  setVoxel(WIDTH-1, WIDTH-1, WIDTH-1);
  strip.show();
  delay(wait);    
  
} // effectBoxZoom


// effect_boxside_randsend_parallel - move dots from one side to the other
void effect_boxside_randsend_parallel (char axis, int dir, int wait) {
  int i;
  int done;
  byte cubepos[WIDTH*WIDTH];
  byte pos[WIDTH*WIDTH];
  int notdone = 1;
  int notdone2 = 1;
  int sent = 0;

  // initialize the positions to zero
  for (i=0; i<WIDTH*WIDTH; i++) pos[i] = 0;

  // loop until all the pixels are moved
  while (notdone) {

    // find one more dot to send, generating random positions until one is found
    notdone2 = 1;
    while (notdone2 && sent<(WIDTH*WIDTH)) {
      i = rand()%(WIDTH*WIDTH);
      if (pos[i] == 0) {
        sent++;
        pos[i] += 1;
        notdone2 = 0;
      }
    }

    // load the array to draw based on the direction
    for (i=0; i<WIDTH*WIDTH; i++) {
      if (dir == 0) {
        cubepos[i] = pos[i];  // move the positive direction - away from the axis
      } else {
        cubepos[i] = ((WIDTH - 1)-pos[i]);  // move towards the axis - reverse direction
      }
    }

    // draw the cube
    draw_positions_axis(axis,cubepos);
    delay(wait);

    // move all the positions forward one, stopping at the end, and counting done rows
    done = 0;
    for (i=0; i<WIDTH*WIDTH; i++) {
      if (pos[i] > 0 && pos[i] <(WIDTH - 1)) pos[i] += 1;
      if (pos[i] == (WIDTH - 1)) done++;
    }

    // set a flag to quit if all rows are complete
    if (done == WIDTH*WIDTH) notdone = 0;

  }

} // effect_boxside_randsend_parallel


// ==========================================================================================
//   Draw functions
// ==========================================================================================

// set all pixels to the given color
void colorFill(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
} // colorFill


// set all pixels to the given color and display them
void colorWipe(uint32_t c) {
  colorFill(c);
  strip.show();
} // colorWipe


// Set a single voxel to ON
void setVoxel(int x, int y, int z) {
  strip.setPixelColor(getPixelNum(x, y, z), strip.Color(0, 0, 255));
} // setVoxel


// Set a single voxel to OFF
void clrVoxel(int x, int y, int z) {
    strip.setPixelColor(getPixelNum(x, y, z), strip.Color(0, 0, 0));
} // clrVoxel


// compute the pixel number from the x, y, and z coord
// this depends on the geometry of the cube
int getPixelNum(int x, int y, int z) {
   int i;
   
    i = x * WIDTH * WIDTH;
    if ((x % 2) == 0) {
      i += y * WIDTH;
      if ((y % 2) == 0) {
        i += z;
      } else {
        i += (WIDTH - 1) - z;
      }
    } else {
      i += ((WIDTH - 1) - y) * WIDTH;
      if ((y % 2) == 0) {
        i += (WIDTH - 1) - z;
      } else {
        i += z;
      }
    }

   return i;
} // getPixelNum


// getPixelColorAt - get the color at the given coordinate
uint32_t getPixelColorAt(int x, int y, int z){
  return strip.getPixelColor(getPixelNum(x, y, z));
}

// Sets all voxels along a Y/Z plane at a given point on axis X
void setplane_x (int x) {
  int y, z;

  if (x>=0 && x<WIDTH) {
    for (z=0;z<WIDTH;z++) {
      for (y=0;y<WIDTH;y++) {
        setVoxel(x,y,z);
      }
    }
  }
} // setplane_x


// Sets all voxels along a X/Z plane at a given point on axis Y
void setplane_y (int y) {
  int x, z;

  if (y>=0 && y<WIDTH) {
    for (z=0;z<WIDTH;z++) {
      for (x=0;x<WIDTH;x++) {
        setVoxel(x,y,z);
      }
    }
  }
} // setplane_y


// Sets all voxels along a X/Y plane at a given point on axis Z
void setplane_z (int z) {
  int x, y;

  if (z>=0 && z<WIDTH) {
    for (y=0;y<WIDTH;y++) {
      for (x=0;x<WIDTH;x++) {
        setVoxel(x,y,z);
      }
    }
  }
} // setplane_z


// sets the given plane
void setplane (char axis, unsigned char i)
{
  switch (axis)
  {
  case AXIS_X:
    setplane_x(i);
    break;

  case AXIS_Y:
    setplane_y(i);
    break;

  case AXIS_Z:
    setplane_z(i);
    break;
  }
}


// clrplane_x - Clears voxels in the given x plane
void clrplane_x (int x) {
  int y, z;

  if (x>=0 && x<WIDTH) {
    for (z=0;z<WIDTH;z++) {
      for (y=0;y<WIDTH;y++) {
        clrvoxel(x,y,z);
      }
    }
  }
} // clrplane_x


// clrplane_y - Clears voxels in the given y plane
void clrplane_y (int y) {
  int x, z;

  if (y>=0 && y<WIDTH) {
    for (z=0;z<WIDTH;z++) {
      for (x=0;x<WIDTH;x++) {
        clrvoxel(x,y,z);
      }
    }
  }
} // clrplane_y


// clrplane_z - Clears voxels in the given z plane
void clrplane_z (int z) {
  int x, y;

  if (z>=0 && z<WIDTH) {
    for (x=0;x<WIDTH;x++) {
      for (y=0;y<WIDTH;y++) {
        clrvoxel(x,y,z);
      }
    }
  }
} // clrplane_z


// clrplane - clear the given plane
void clrplane(char axis, unsigned char i) {
  switch (axis) {
    case AXIS_X:
      clrplane_x(i);
      break;

    case AXIS_Y:
      clrplane_y(i);
      break;

    case AXIS_Z:
      clrplane_z(i);
      break;
  }
} // clrplane


// shift_z_down - shift the cube down the z axis
void shift_z_down() {
  int x, y, z;

  for (z=0; z<(WIDTH-1); z++) {
    for (x=0; x<WIDTH; x++) {
      for (y=0; y<WIDTH; y++) {
        strip.setPixelColor(getPixelNum(x, y, z), getPixelColorAt(x, y, z+1));
      }
    }
  }
  strip.show();
} // shift_z_down


// Draw a box with all walls drawn and all voxels inside set
void box_filled(int x1, int y1, int z1, int x2, int y2, int z2) {
  int ix, iy, iz;
  
  for (ix=x1; ix<=x2; ix++) {
    for (iy=y1; iy<=y2; iy++) {
      for (iz=z1; iz<=z2; iz++) {
        setVoxel(ix, iy, iz);
      } //iz
    } // iy
  } // ix
  strip.show();

} // box_filled


// draw_positions_axis - draw a dot at each position in the matrix - 0 is the starting position, and will light up a fill slice
void draw_positions_axis(char axis, byte positions[WIDTH*WIDTH]) {
  int x, y, p;

  colorWipe(strip.Color(0, 0, 0)); // Black

  for (x=0; x<WIDTH; x++) {
    for (y=0; y<WIDTH; y++) {
      p = positions[(x*WIDTH)+y];

      if (axis == AXIS_Z)
        setVoxel(x,y,p);

      if (axis == AXIS_Y)
        setVoxel(x,p,y);

      if (axis == AXIS_X)
        setVoxel(p,y,x);
    }
  }

  strip.show();
} // draw_positions_axis


// Set a single voxel to ON
void clrvoxel(int x, int y, int z)
{
  int i;
  
  strip.setPixelColor(getPixelNum(x, y, z), strip.Color(0, 0, 0));
  strip.show();
}

// this is an old function that should be removed
// Delay loop.
// This is not calibrated to milliseconds,
// but we had allready made to many effects using this
// calibration when we figured it might be a good idea
// to calibrate it.
void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 90 ; y++){
      for ( z = 0 ; z < 6 ; z++){
        asm volatile ("nop");
      }
    }
  }
}



