#include "i_video.h"
#include "doomstat.h"
#include "doomtype.h"
#include "lprintf.h"
#include "w_wad.h"
#include "v_video.h"
#include "d_event.h"
#include "d_main.h"
#include "z_zone.h"
#include "doom_display.h"
#include <xcore/chanend.h>
#include <xcore/select.h>


extern "C" {
  #include "ps2.h"
  extern chanend_t g_c_ps2;
}

int use_vsync = 0; // Included not to break m_misc, but not relevant to SDL
int leds_always_off = 0; // Expected by m_misc, not relevant

int I_DoomCode2ScanCode(int c)
{
  __builtin_trap();
}

int I_ScanCode2DoomCode(int c)
{
  __builtin_trap();
}

typedef enum xdoom_event_type{
  SDL_KEYDOWN,
  SDL_KEYUP,
  SDL_QUIT
}xdoom_event_type;

typedef struct xdoom_event{
  xdoom_event_type type;
  int key;
}xdoom_event;

static int I_TranslateKey(int key)
{
  int rc = key;
  // Optional key translation stuff. Hope not needed as using PS2

  return rc;

}

static void I_GetEvent(xdoom_event *Event)
{
  event_t event;

  switch (Event->type) {
  case SDL_KEYDOWN:
    event.type = ev_keydown;
    event.data1 = I_TranslateKey(Event->key);
    D_PostEvent(&event);
    break;

  case SDL_KEYUP:
  {
    event.type = ev_keyup;
    event.data1 = I_TranslateKey(Event->key);
    D_PostEvent(&event);
  }
  break;

  // No mouse support

  case SDL_QUIT:
    // S_StartSound(NULL, sfx_swtchn);
    // M_QuitDOOM(0);
    printf("SDL_QUIT\n");
    break;

  default:
    break;
  }
}

int X_PollEvent(xdoom_event *Event){
  // Check event queue here from PS2 and post if some keypress has happened

  SELECT_RES(
  CASE_THEN(g_c_ps2, ps2_input_case),
  DEFAULT_THEN(default_case))
  {
  default_case:
    // Do nothing - fallthrough
    break;
  ps2_input_case:
    {
      unsigned action, modifier, key;
      action = chanend_in_byte(g_c_ps2);
      modifier = chanend_in_byte(g_c_ps2);
      key = chanend_in_byte(g_c_ps2);
      unsigned ascii_key = ps2ASCII(modifier, key);
      if (action == PS2_PRESS) {
        printf("Modifiers 0x%02x press %d - %d (%c)\n", modifier, key, ascii_key, ascii_key);
      } else if (action == PS2_RELEASE) {
        printf("Modifiers 0x%02x release %d - %d\n", modifier, key, ascii_key);
      }
    }
    break;
  }

  return 0; // No event
}

void I_StartTic (void)
{
  printf("I_StartTic\n");

  xdoom_event Event;
  while ( X_PollEvent(&Event) )
    I_GetEvent(&Event);

}

void I_StartFrame (void)
{
}

void I_ShutdownGraphics(void)
{
}

void I_UpdateNoBlit (void)
{
}

static inline boolean I_SkipFrame(void)
{
  static int frameno;

  frameno++;
  switch (gamestate) {
#pragma fallthrough
  case GS_LEVEL:
    if (!paused)
      return false;
  default:
    // Skip odd frames
    return (frameno & 1) ? true : false;
  }
}

void I_FinishUpdate (void)
{
  if (I_SkipFrame())
    return;

  doom_display_write(screens[0]);
}

void I_ReadScreen (byte* scr)
{
  doom_display_read(scr);
}

void I_SetPalette (int pal)
{
  static uint16_t *colours;
  static int cachedgamma;
  static size_t num_pals;

  if (colours == NULL || cachedgamma != usegamma) {
    int lump = W_GetNumForName("PLAYPAL");
    const byte *palette = (const byte *)W_CacheLumpNum(lump);
    const byte *const gtable = gammatable[cachedgamma = usegamma];
    int i;

    num_pals = W_LumpLength(lump) / (3*256);
    num_pals *= 256;

    if (!colours) {
      // First call - allocate and prepare colour array
      colours = (uint16_t *)malloc(sizeof(*colours) * num_pals);
    }

    // set the colormap entries
    for (i = 0 ; i < num_pals ; i++) {
      byte r = gtable[palette[0]];
      byte g = gtable[palette[1]];
      byte b = gtable[palette[2]];
      colours[i] = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
      palette += 3;
    }
  
    W_UnlockLumpNum(lump);
    num_pals /= 256;
  }

#ifdef RANGECHECK
  if (pal >= num_pals) 
    I_Error("I_UploadNewPalette: Palette number out of range (%d>=%d)", 
	    pal, (int)num_pals);
#endif

  // store the colors to the current display
  doom_display_set_palette(colours + 256 * pal);
}

void I_PreInitGraphics(void)
{
}

void I_SetRes(unsigned int width, unsigned int height)
{
  printf("width %u height %u\n", width, height);
  if (width != SCREEN_WIDTH || height != SCREEN_HEIGHT)
    __builtin_trap();
}

void I_InitGraphics(void)
{
  lprintf(LO_INFO,"I_InitGraphics: xCORE\n");
}

