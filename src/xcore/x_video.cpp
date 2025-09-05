#include "i_video.h"
#include "doomstat.h"
#include "doomdef.h"
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
  int rc = 0;
// Convert an ASCII char or special scan code into a Doom event_t.
    switch (key)
    {
        // --- Arrow keys (non-ASCII, usually from scan codes) ---
        case 0x34: rc = KEYD_LEFTARROW;  break;
        case 0x36: rc = KEYD_RIGHTARROW; break;
        case 0x38: rc = KEYD_UPARROW;    break;
        case 0x32: rc = KEYD_DOWNARROW;  break;

        // --- Control keys ---
        case 0x1b: rc = KEYD_ESCAPE;    break; // ESC
        case 0x0d: rc = KEYD_ENTER;     break; // Enter
        case 0x09: rc = KEYD_TAB;       break; // Tab
        case 0x7f: rc = KEYD_BACKSPACE; break; // Backspace
        case 0x20: rc = KEYD_SPACEBAR;  break; // Space

        // --- Function keys (scan codes, not ASCII) ---
        case 0x80: rc = KEYD_F1;  break;
        case 0x81: rc = KEYD_F2;  break;
        case 0x82: rc = KEYD_F3;  break;
        case 0x83: rc = KEYD_F4;  break;
        case 0x84: rc = KEYD_F5;  break;
        case 0x85: rc = KEYD_F6;  break;
        case 0x86: rc = KEYD_F7;  break;
        case 0x87: rc = KEYD_F8;  break;
        case 0x88: rc = KEYD_F9;  break;
        case 0x89: rc = KEYD_F10; break;

        // --- Page / Home / Insert / Delete ---
        case 0x95: rc = KEYD_PAGEUP;   break;
        case 0x96: rc = KEYD_PAGEDOWN;   break;
        case 0x97: rc = KEYD_HOME;   break;
        case 0x98: rc = KEYD_END;    break;
        case 0x99: rc = KEYD_INSERT;    break;
        case 0x9a: rc = KEYD_DEL;    break;

        // --- Default: printable ASCII maps directly ---
        default:
            rc = key;
            break;
    }

  // printf("rc: 0x%x\n", rc);
  return rc;
}

static void I_GetEvent(xdoom_event *Event)
{
  event_t event;

  printf("I_GetEvent: %d 0x%x\n", Event->type, Event->key);

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
      // printf("key: 0x%x asciikey: 0x%x raw modifer: 0x%x\n", key, ascii_key, modifier);
      if (action == PS2_PRESS) {
        Event->type = SDL_KEYDOWN;
        // printf("Modifiers 0x%02x press %d - 0x%x (%c)\n", modifier, key, ascii_key, ascii_key);
      } else if (action == PS2_RELEASE) {
        Event->type = SDL_KEYUP;
        // printf("Modifiers 0x%02x release %d - 0x%x (%c)\n", modifier, key, ascii_key, ascii_key);
      }
      Event->key = ascii_key;
    }
    return 1;
    break;
  }

  return 0; // No event
}

void I_StartTic (void)
{
  xdoom_event Event;
  while ( X_PollEvent(&Event) ){
    I_GetEvent(&Event);
  }

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

