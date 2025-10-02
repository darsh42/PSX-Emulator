#include <assert.h>
#include <stdio.h>

#define SPU_PRIVATE
#define SPU_SECTORS
#include "spu.h"
#define CDROM_SECTORS
#include "cdrom.h"
#include "memory.h"
#include "system.h"

struct spu spu;

uint32_t read_spu_voice( uint32_t address )
{
    uint32_t data, voice;

    voice   = (address & 0x000000F0) >> 4;
    address = (address & 0xfffffe0f);

    switch (address) {
    case (spu_voice_volume_left_base         ): break;
    case (spu_voice_volume_right_base        ): break;
    case (spu_voice_adpcm_sample_rate_base   ): data = spu.adpcm_sample_rate[voice];    break;
    case (spu_voice_adpcm_start_address_base ): data = spu.adpcm_start_address[voice];  break;
    case (spu_voice_adsr_lower_base          ): break;
    case (spu_voice_adsr_upper_base          ): break;
    case (spu_voice_adsr_current_volume_base ): break;
    case (spu_voice_adpcm_repeat_address_base): data = spu.adpcm_repeat_address[voice]; break;
    default:
        assert(0 && "unhandled spu voice register");
        break;
    }

    // TRACE_SPU("read_spu_voice", "address: %08x | data: %08x\n", address, data);

    return data;
}

void write_spu_voice( uint32_t address, uint32_t data)
{
    /* check for voice registers */

    uint32_t voice;

    voice   = (address & 0x000000F0) >> 4;
    address = (address & 0xfffffe0f);

    switch (address) {
    case (spu_voice_volume_left_base         ): break;
    case (spu_voice_volume_right_base        ): break;
    case (spu_voice_adpcm_sample_rate_base   ): spu.adpcm_sample_rate[voice]   = data;  break;
    case (spu_voice_adpcm_start_address_base ): spu.adpcm_start_address[voice] = data;  break;
    case (spu_voice_adsr_lower_base          ): spu.adsr[voice] |= (data & 0x0000ffff); break;
    case (spu_voice_adsr_upper_base          ): spu.adsr[voice] |= (data & 0xffff0000); break;
    case (spu_voice_adsr_current_volume_base ): spu.adsr_volume[voice] = data;          break;
    case (spu_voice_adpcm_repeat_address_base): spu.adpcm_repeat_address[voice] = data; break;
    default:
        assert(0 && "unhandled spu voice register");
        break;
    }

    // TRACE_SPU("write_spu_voice", "address: %08x | data: %08x\n", address, data);
}

uint32_t read_spu( uint32_t address )
{
    uint32_t data = 0;

    switch (address) {
    case (spu_main_volume_left_right             ): break;
    case (spu_reverb_output_volume_left_right    ): break;
    case (spu_voice_key_on                       ): data = spu.kon;           break;
    case (spu_voice_key_off                      ): data = spu.koff;          break;
    case (spu_channel_fm                         ): break;
    case (spu_channel_noise                      ): break;
    case (spu_channel_reverb                     ): break;
    case (spu_channel_status                     ): break;
    case (spu_sram_reverb_work_area_start_address): break;
    case (spu_sram_irq_address                   ): break;
    case (spu_sram_data_transfer_address         ): data = spu.sram_address;  break;
    case (spu_sram_data_transfer_fifo            ):
        /* read the data at the current sram pointer */
        memory_read_sram(spu.sram_current, &data, 2);
        /* increment the current sram pointer */
        spu.sram_current += 2;
        break;
    case (spucnt                                 ): data = spu.spucnt.value;  break;
    case (spu_sram_data_transfer_control         ): data = spu.sram_control;  break;
    case (spustat                                ): data = spu.spustat.value; break;
    case (spu_cd_volume_left_right               ): break;
    case (spu_extern_volume_left_right           ): break;
    case (spu_current_main_volume_left_right     ): break;
    default:
        break;
    }

    // TRACE_SPU("read_spu ", "address: %08x | data: %08x\n", address, data);

    return data;
}

void write_spu( uint32_t address, uint32_t data )
{
    switch (address) {
    case (spu_main_volume_left_right             ): break;
    case (spu_reverb_output_volume_left_right    ): break;
    case (spu_voice_key_on                       ): spu.kon  = data;         break;
    case (spu_voice_key_off                      ): spu.koff = data;         break;
    case (spu_channel_fm                         ): break;
    case (spu_channel_noise                      ): break;
    case (spu_channel_reverb                     ): break;
    case (spu_channel_status                     ): break;
    case (spu_sram_reverb_work_area_start_address): break;
    case (spu_sram_irq_address                   ): break;
    case (spu_sram_data_transfer_address         ):
        /* store the sram base address */
        spu.sram_address = data;
        /* load the sram current data pointer */
        spu.sram_current = data << 3;
        break;
    case (spu_sram_data_transfer_fifo            ):
        /* write data into sram */
        memory_write_sram(spu.sram_current, data, 2);
        /* increment the current data pointer */
        spu.sram_current += 2;
        break;
    case (spucnt                                 ): spu.spucnt.value = data;  break;
    case (spu_sram_data_transfer_control         ): spu.sram_control = data;  break;
    case (spustat                                ): spu.spustat.value = data; break;
    case (spu_cd_volume_left_right               ): break;
    case (spu_extern_volume_left_right           ): break;
    case (spu_current_main_volume_left_right     ): break;
    default:
        break;
    }

    // TRACE_SPU("write_spu", "address: %08x | data: %08x\n", address, data);
}

#define SIGN_MSK(   b) (1U << ((b) - 1))
#define SIGN_EXT(x, b) (((x) ^ SIGN_MSK((b))) - SIGN_MSK((b)))

#define CLAMP(x, hi, lo) ((x < lo) ? lo: (x > hi) ? hi: x)
static inline void spu_decode_samples(struct spu_adpcm_sector *sector,
                                      int16_t *decode_buffer,
                                      int16_t *old,
                                      int16_t *older) {
    TRACE_SPU("spu_decode_samples", "\nsector:\
                                     \n\tshift:  %d\
                                     \n\tfilter: %d\
                                     \n\tdata [%02x, %02x, %02x, %02x]\n",
            sector->shift, sector->filter, sector->data[0], sector->data[1], 
                                           sector->data[2], sector->data[3]);

    static int32_t pos_adpcm_table[] = {0, +60, +115, +98, +122};
    static int32_t neg_adpcm_table[] = {0,   0,  -52, -55,  -60};

    int8_t shift  = sector->shift;
    int8_t filter = sector->filter;
    int32_t f0 = pos_adpcm_table[filter];
    int32_t f1 = neg_adpcm_table[filter];

    /* shift 13-15 treated as 9 */
    if (shift > 12)
        shift = 9;

    /* decoding algorithm */
    for (uint32_t n = 0; n < 14; n++) {
        int32_t msb, lsb, _old, _older;

        /* sign extend old samples */
        _old   = SIGN_EXT(*old,   16);
        _older = SIGN_EXT(*older, 16);

        /* sign extend compressed samples */
        lsb = SIGN_EXT(sector->data[n] >> 0, 4);
        msb = SIGN_EXT(sector->data[n] >> 4, 4);

        /* apply shift mask */
        lsb <<= (12 - shift);
        msb <<= (12 - shift);

        /* calculate the sample */
        lsb = lsb + (32 + f0 * _old + f1 * _older) / 64; 
        msb = msb + (32 + f0 *  lsb + f1 * _old  ) / 64; 

        lsb = CLAMP(lsb, +0x7FFF, -0x8000);
        msb = CLAMP(msb, +0x7FFF, -0x8000);

        /* populate voice sample buffers */
        decode_buffer[2*n + 0] = (int16_t) lsb;
        decode_buffer[2*n + 1] = (int16_t) msb;

        /* set new old and older samples */
        *older = (int16_t) lsb;
        *old   = (int16_t) msb;
    }
}

static inline void spu_decode_block( int voice ) {
    struct spu_adpcm_sector *sector;

    TRACE_SPU("spu_decoce_block", "(v%d) adpcm_current_address: %08x\n", 
            voice, spu.adpcm_current_address[voice]);

    memory_read_sram_sector(
            spu.adpcm_current_address[voice], &sector);

    /* populate voice samples buffer */
    spu_decode_samples(sector,
                       spu.decode_buffers[voice],
                       &spu.decode_history_old[voice],
                       &spu.decode_history_older[voice]);


    /* sets the repeat address if current block has loop start set */
    if (sector->loop_start) {
        spu.adpcm_repeat_address[voice] = 
            spu.adpcm_current_address[voice] >> 3;
    }

    /* jumps to adpcm repeat address if current block has loop end set */
    if (sector->loop_end) {
        spu.adpcm_current_address[voice] = 
            spu.adpcm_repeat_address[voice] << 3;

        /* silence voice (volume 0) */
        if (!sector->loop_repeat) {
            spu.endx |= (1 << voice);
            spu.koff |= (1 << voice); // BUG: seems like adsr related
        }
    } else {
        spu.adpcm_current_address[voice] += 16;
    }
}

static inline int16_t spu_service_voice(uint32_t voice)
{
    /* check for keyon */
    if (spu.kon & (1 << voice)) {
        /* copy start address to current address */
        spu.adpcm_current_address[voice] = 
            spu.adpcm_start_address[voice] << 3;

        /* reset pitch counter */
        spu.pitch_counter[voice] = 0;

        /* reset decode buffer index */
        spu.decode_buffers_index[voice] = 0;

        /* pre-fill the buffer */
        spu_decode_block(voice);

        /* clear keyon and endx flag */
        spu.endx &= ~(1 << voice);
        spu.kon  &= ~(1 << voice); // BUG: seems adsr related
    }

    /* clamp and add the sample rate to the pitch counter */
    spu.pitch_counter[voice] += spu.adpcm_sample_rate[voice];

    /* pitch counter determines how many samples are stepped through *
     * during each spu clock                                         */
    while (spu.pitch_counter[voice] > 0x1000) {
        /* decrement the pitch counter */
        spu.pitch_counter[voice] -= 0x1000;
        spu.decode_buffers_index[voice]++;

        /* if the decode buffer index reaches the end, decode new samples and *
         * set index to 0                                                     */
        if (spu.decode_buffers_index[voice] == 28) {
            spu.decode_buffers_index[voice] = 0;
            spu_decode_block(voice);
        }
    }

    return spu.decode_buffers[voice][spu.decode_buffers_index[voice]];

    /* add adsr handling */

    /* add volume and panning */
}

#define MIX_SHIFT 4
static inline int16_t spu_mix_samples(int32_t mix) {
    int32_t sample = mix >> MIX_SHIFT;

    sample = (sample < INT16_MIN) ? INT16_MIN: sample;
    sample = (sample > INT16_MAX) ? INT16_MAX: sample;

    return sample;
}

void task_spu( void ) {
    int32_t acc = 0;

    for (uint32_t v = 0; v < 24; v++)
        acc += spu_service_voice(v);

    int16_t current_sample = 
        spu_mix_samples(acc);

    system_audio_push_sample(current_sample, 
                             current_sample);
}
