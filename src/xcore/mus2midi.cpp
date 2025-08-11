#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>



typedef uint8_t BYTE;
typedef uint32_t DWORD;

#define MAKE_ID(a,b,c,d) (((DWORD)(d)<<24)|((DWORD)(c)<<16)|((DWORD)(b)<<8)|(DWORD)(a))

// MIDI event constants (subset)
#define MIDI_NOTEOFF   0x80
#define MIDI_NOTEON    0x90
#define MIDI_PITCHBEND 0xE0
#define MIDI_CTRLCHANGE 0xB0
#define MIDI_PRGMCHANGE 0xC0
#define MIDI_META      0xFF

#define MIDI_META_EOT  0x2F
#define MIDI_META_SSPEC 0x7F

// MUS events (subset)
#define MUS_NOTEOFF    0x00
#define MUS_NOTEON     0x10
#define MUS_PITCHBEND  0x20
#define MUS_SYSEVENT   0x30
#define MUS_CTRLCHANGE 0x40
#define MUS_SCOREEND   0x60

struct MUSHeader
{
    DWORD Magic;
    uint16_t ScoreLen;
    uint16_t SongStart;
    uint16_t NumChans;
    uint16_t SecChannels;
    uint16_t Pad1;
    uint16_t Pad2;
    uint16_t Pad3;
    uint16_t Pad4;
};

static const BYTE StaticMIDIhead[] =
{
    'M','T','h','d', 0,0,0,6,
    0,0, 0,1,
    0,70,
    'M','T','r','k', 0,0,0,0,
    0,255,81,3,0x07,0xA1,0x20
};

static const BYTE CtrlTranslate[15] =
{
    0, 0, 1, 7, 10, 11, 91, 93,
    64, 67, 120, 123, 126, 127, 121
};

// Read 16-bit little endian
static uint16_t LittleShort(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

// Read variable-length value from buffer, returns bytes read
static size_t ReadVarLen(const BYTE *buf, int *time_out)
{
    int time = 0;
    size_t ofs = 0;
    BYTE t;
    do {
        t = buf[ofs++];
        time = (time << 7) | (t & 0x7F);
    } while (t & 0x80);
    *time_out = time;
    return ofs;
}

// Write variable-length value into outBuf at position pos, checking bounds.
// Returns bytes written, or 0 if outBufSize exceeded.
static size_t WriteVarLen(BYTE *outBuf, size_t pos, size_t outBufSize, int time)
{
    long buffer = time & 0x7F;
    size_t bytes = 1;

    while ((time >>= 7) > 0)
    {
        buffer = (buffer << 8) | 0x80 | (time & 0x7F);
        bytes++;
    }

    if (pos + bytes > outBufSize)
        return 0;

    for (size_t i = 0; i < bytes; i++)
    {
        outBuf[pos + bytes - 1 - i] = buffer & 0xFF;
        buffer >>= 8;
    }

    return bytes;
}

// Main conversion function, returns number of bytes written or 0 on error
// outBuf must be preallocated by caller
extern "C" size_t ProduceMIDIToBuffer(const BYTE *musBuf, size_t musSize, BYTE *outBuf, size_t outBufSize)
{
    if (!musBuf || !outBuf || musSize < sizeof(MUSHeader) || outBufSize < sizeof(StaticMIDIhead)) {
        printf("Error: Invalid input buffers or sizes\n");
        return 0;
    }

    const MUSHeader *musHead = (const MUSHeader*)musBuf;

    if (musHead->Magic != MAKE_ID('M','U','S',0x1a)) {
        return 0;
    }

    size_t songStart = LittleShort((const uint8_t*)&musHead->SongStart);
    size_t songLen = LittleShort((const uint8_t*)&musHead->ScoreLen);
    size_t numChans = LittleShort((const uint8_t*)&musHead->NumChans);

    // printf("songStart: %ld songLen: %ld numChans: %ld\n", songStart, songLen, numChans);

    if (numChans > 15) {
        printf("Error: Number of channels (%zu) exceeds 15\n", numChans);
        return 0;
    }
    if (songStart + songLen > musSize) {
        printf("Error: Song data extends beyond input buffer (start %zu + len %zu > musSize %zu)\n", songStart, songLen, musSize);
        return 0;
    }

    size_t pos = 0;

    // Copy MIDI header
    memcpy(outBuf, StaticMIDIhead, sizeof(StaticMIDIhead));
    pos += sizeof(StaticMIDIhead);

    // Setup variables
    size_t mus_p = songStart;
    size_t maxmus_p = songStart + songLen;
    BYTE lastVel[16];
    memset(lastVel, 100, sizeof(lastVel));
    BYTE chanUsed[16] = { 0 };
    BYTE event = 0, status = 0;
    int deltaTime = 0;

    while (mus_p < maxmus_p && (event & 0x70) != MUS_SCOREEND)
    {
        if (mus_p >= maxmus_p) {
            printf("Error: Unexpected end of MUS data while reading event\n");
            break;
        }
        event = musBuf[mus_p++];

        BYTE t = 0;
        if ((event & 0x70) != MUS_SCOREEND)
        {
            if (mus_p >= maxmus_p) {
                printf("Error: Unexpected end of MUS data while reading event parameter\n");
                break;
            }
            t = musBuf[mus_p++];
        }

        int channel = event & 0x0F;
        if (channel == 15) channel = 9;
        else if (channel >= 9) channel++;

        // If first time use channel, set volume
        if (!chanUsed[channel])
        {
            if (pos + 4 > outBufSize) {
                printf("Error: Output buffer too small setting initial channel volume\n");
                return 0;
            }
            chanUsed[channel] = 1;
            outBuf[pos++] = 0;
            outBuf[pos++] = 0xB0 | (BYTE)channel;
            outBuf[pos++] = 7;
            outBuf[pos++] = 127;
        }

        BYTE midStatus = (BYTE)channel;
        BYTE mid1 = 0, mid2 = 0;
        BYTE midArgs = 0;
        bool no_op = false;

        switch (event & 0x70)
        {
            case MUS_NOTEOFF:
                midStatus |= MIDI_NOTEOFF;
                mid1 = t & 0x7F;
                mid2 = 64;
                break;

            case MUS_NOTEON:
                midStatus |= MIDI_NOTEON;
                mid1 = t & 0x7F;
                if (t & 0x80)
                {
                    if (mus_p >= maxmus_p) {
                        printf("Error: Unexpected end of MUS data reading velocity\n");
                        return 0;
                    }
                    lastVel[channel] = musBuf[mus_p++] & 0x7F;
                }
                mid2 = lastVel[channel];
                break;

            case MUS_PITCHBEND:
                midStatus |= MIDI_PITCHBEND;
                mid1 = (t & 1) << 6;
                mid2 = (t >> 1) & 0x7F;
                break;

            case MUS_SYSEVENT:
                if (t < 10 || t > 14)
                {
                    no_op = true;
                }
                else
                {
                    midStatus |= MIDI_CTRLCHANGE;
                    mid1 = CtrlTranslate[t];
                    mid2 = (t == 12) ? (BYTE)numChans : 0;
                }
                break;

            case MUS_CTRLCHANGE:
                if (t == 0)
                {
                    midArgs = 1;
                    midStatus |= MIDI_PRGMCHANGE;
                    if (mus_p >= maxmus_p) {
                        printf("Error: Unexpected end of MUS data reading program change\n");
                        return 0;
                    }
                    mid1 = musBuf[mus_p++] & 0x7F;
                }
                else if (t > 0 && t < 10)
                {
                    midStatus |= MIDI_CTRLCHANGE;
                    mid1 = CtrlTranslate[t];
                    if (mus_p >= maxmus_p) {
                        printf("Error: Unexpected end of MUS data reading control change value\n");
                        return 0;
                    }
                    mid2 = musBuf[mus_p++];
                }
                else
                {
                    no_op = true;
                }
                break;

            case MUS_SCOREEND:
                midStatus = MIDI_META;
                mid1 = MIDI_META_EOT;
                mid2 = 0;
                break;

            default:
                printf("Error: Unknown MUS event type 0x%X (score end is 0x%x)\n", event & 0x70, MUS_SCOREEND);
                return 0;
        }

        if (no_op)
        {
            midStatus = MIDI_META;
            mid1 = MIDI_META_SSPEC;
            mid2 = 0;
        }

        // Write deltaTime as varlen
        size_t varLenBytes = WriteVarLen(outBuf, pos, outBufSize, deltaTime);
        if (varLenBytes == 0) {
            printf("Error: Output buffer too small while writing deltaTime\n");
            return 0;
        }
        pos += varLenBytes;

        if (midStatus != status)
        {
            if (pos >= outBufSize) {
                printf("Error: Output buffer too small writing status byte\n");
                return 0;
            }
            status = midStatus;
            outBuf[pos++] = status;
        }

        if (pos >= outBufSize) {
            printf("Error: Output buffer too small writing mid1 (%u)\n", outBufSize);
            return 0;
        }
        outBuf[pos++] = mid1;

        if (midArgs == 0)
        {
            if (pos >= outBufSize) {
                printf("Error: Output buffer too small writing mid2\n");
                return 0;
            }
            outBuf[pos++] = mid2;
        }

        if (event & 0x80)
        {
            if (mus_p >= maxmus_p) {
                printf("Error: Unexpected end of MUS data reading delta time\n");
                return 0;
            }
            int dt = 0;
            size_t adv = ReadVarLen(musBuf + mus_p, &dt);
            if (mus_p + adv > maxmus_p) {
                printf("Error: Delta time extends beyond MUS buffer\n");
                return 0;
            }
            mus_p += adv;
            deltaTime = dt;
        }
        else
            deltaTime = 0;
    }

    // Fix track length in MIDI header (bytes 18-21)
    int32_t trackLen = int32_t(pos) - 22;
    if (trackLen < 0) {
        printf("Error: Track length calculation negative\n");
        return 0;
    }

    if (pos < 22) {
        printf("Error: Output buffer too small, less than MIDI header size\n");
        return 0;
    }

    outBuf[18] = (trackLen >> 24) & 0xFF;
    outBuf[19] = (trackLen >> 16) & 0xFF;
    outBuf[20] = (trackLen >> 8) & 0xFF;
    outBuf[21] = trackLen & 0xFF;

    return pos;
}




// (Assuming ProduceMIDIToBuffer is declared and implemented as above)
#if TEST_MUS2MID

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: mus2mid inputfile.mus\n");
        return 1;
    }

    const char *inputPath = argv[1];

    // Open .mus file
    FILE *inFile = fopen(inputPath, "rb");
    if (!inFile)
    {
        printf("Error: Could not open input file '%s'\n", inputPath);
        return 1;
    }

    // Determine file size
    fseek(inFile, 0, SEEK_END);
    long fileSize = ftell(inFile);
    if (fileSize <= 0)
    {
        fclose(inFile);
        printf("Error: Input file is empty or invalid size.\n");
        return 1;
    }
    fseek(inFile, 0, SEEK_SET);

    // Read entire file into memory
    BYTE *musData = new BYTE[fileSize];
    if (fread(musData, 1, fileSize, inFile) != (size_t)fileSize)
    {
        fclose(inFile);
        delete[] musData;
        printf("Error: Could not read input file completely.\n");
        return 1;
    }
    fclose(inFile);

    // printf("Read midi %s of size %ld.\n", inputPath,  fileSize);

    // Allocate output buffer for MIDI data (64 KB should be enough for most MUS files)
    const size_t midiBufferSize = 72 * 1024; // 71 min output for D_E1M8.mus
    BYTE *midiBuffer = new BYTE[midiBufferSize];

    // Convert MUS to MIDI in memory
    size_t midiSize = ProduceMIDIToBuffer(musData, fileSize, midiBuffer, midiBufferSize);

    delete[] musData;

    if (midiSize == 0)
    {
        delete[] midiBuffer;
        printf("Error: Conversion failed or output buffer too small.\n");
        return 1;
    }

    // Construct output filename by replacing extension with .mid
    std::string outPath = inputPath;
    size_t dotPos = outPath.find_last_of('.');
    if (dotPos != std::string::npos)
        outPath = outPath.substr(0, dotPos);
    outPath += ".mid";

    // Write MIDI data to output file
    FILE *outFile = fopen(outPath.c_str(), "wb");
    if (!outFile)
    {
        delete[] midiBuffer;
        printf("Error: Could not open output file '%s' for writing.\n", outPath.c_str());
        return 1;
    }

    size_t written = fwrite(midiBuffer, 1, midiSize, outFile);
    fclose(outFile);
    delete[] midiBuffer;

    if (written != midiSize)
    {
        printf("Error: Could not write complete MIDI data to file.\n");
        return 1;
    }

    printf("Successfully converted '%s' (%ld bytes) to '%s' (%u bytes).\n", inputPath, fileSize, outPath.c_str(), midiSize);
    return 0;
}

#endif
