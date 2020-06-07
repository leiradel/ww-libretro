#ifndef WW_CONFIG_H
#define WW_CONFIG_H

// Maximum number of players.
#define WW_MAX_PLAYERS (4)

// Maximum number of tiles.
#define WW_MAX_TILES (32)

// The maximum *total* number of sprites.
#define WW_MAX_SPRITES (16)

// Screen dimensions for one player.
#define WW_SCREEN_WIDTH  (240)
#define WW_SCREEN_HEIGHT (128)

// The number of video frames per second.
#define WW_FRAME_RATE (60)

// Tile dimensions.
#define WW_TILE_SIZE (16)

// The frequency of the mixed sound.
#define WW_SAMPLE_RATE (44100)

// Number of 16-bit stereo samples per frame. DO NOT CHANGE!
#define WW_SAMPLES_PER_FRAME (WW_SAMPLE_RATE / WW_FRAME_RATE)

// Number of sfxr voices that can play simultaneously.
#define WW_MAX_VOICES (3)

#endif // WW_CONFIG_H
