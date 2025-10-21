#include <assert.h>
#include <stdio.h>

#define SPU_PRIVATE
#define SPU_SECTORS
#include "spu.h"
#define CDROM_SECTORS
#include "cdrom.h"
#include "memory.h"
#include "system.h"

#define TRACE_VOICE                                                     \
    TRACE_SPU("spu_service_voice", "\n\                                 \
            voice(%d):\n\                                               \
                volume: %x\n\                                           \
                sample_rate: %x\n\                                      \
                current_address: %x\n\                                  \
                repeat_address: %x\n\                                   \
                start_address: %x\n\                                    \
                adsr: %x\n\                                             \
                adsr_volume: %x\n\                                      \
                pitch_counter: %x\n", v,                                \
        voice->volume, voice->sample_rate, voice->current_address,      \
        voice->repeat_address, voice->start_address, voice->adsr,       \
        voice->adsr_volume, voice->pitch_counter);  

struct spu spu;

uint32_t read_spu_voice(uint32_t address) {
    uint32_t data, voice;

    voice   = (address - 0x1f801c00) >> 4;
    address = (address & 0xfffffe0f);

    switch (address) {
    case (spu_voice_volume_left_base         ): break;
    case (spu_voice_volume_right_base        ): break;
    case (spu_voice_adpcm_sample_rate_base   ): data = spu.voices[voice].sample_rate;       break;
    case (spu_voice_adpcm_start_address_base ): data = spu.voices[voice].start_address;     break;
    case (spu_voice_adsr_lower_base          ): data = spu.voices[voice].adsr & 0x0000ffff; break;
    case (spu_voice_adsr_upper_base          ): data = spu.voices[voice].adsr & 0xffff0000; break;
    case (spu_voice_adsr_current_volume_base ): data = spu.voices[voice].adsr_volume;       break;
    case (spu_voice_adpcm_repeat_address_base): data = spu.voices[voice].repeat_address;    break;
    default:
        assert(0 && "unhandled spu voice register");
        break;
    }

    // TRACE_SPU("read_spu_voice", "address: %08x | data: %08x\n", address, data);

    return data;
}

void write_spu_voice(uint32_t address, uint32_t data) {
    uint32_t voice;

    voice   = (address - 0x1f801c00) >> 4;
    address = (address & 0xfffffe0f);

    switch (address) {
    case (spu_voice_volume_left_base         ): break;
    case (spu_voice_volume_right_base        ): break;
    case (spu_voice_adpcm_sample_rate_base   ): spu.voices[voice].sample_rate   = data;        break;
    case (spu_voice_adpcm_start_address_base ): spu.voices[voice].start_address = data;        break;
    case (spu_voice_adsr_lower_base          ): spu.voices[voice].adsr |= (data & 0x0000ffff); break;
    case (spu_voice_adsr_upper_base          ): spu.voices[voice].adsr |= (data & 0xffff0000); break;
    case (spu_voice_adsr_current_volume_base ): spu.voices[voice].adsr_volume    = data;       break;
    case (spu_voice_adpcm_repeat_address_base): spu.voices[voice].repeat_address = data;       break;
    default:
        assert(0 && "unhandled spu voice register");
        break;
    }

    // TRACE_SPU("write_spu_voice", "address: %08x | data: %08x\n", address, data);
}

uint32_t read_spu(uint32_t address) {
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

void write_spu(uint32_t address, uint32_t data) {
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
static inline int32_t signextend(int32_t val, int32_t signbit) {
    return (val ^ signbit) - signbit;
}
void spu_decode_samples(struct spu_adpcm_sector *sector,
                        int16_t *decode_buffer,
                        int16_t *old,
                        int16_t *older) {
    static int32_t pos_adpcm_table[] = {0, +60, +115, +98, +122};
    static int32_t neg_adpcm_table[] = {0,   0,  -52, -55,  -60};

    int8_t shift  = sector->shift;
    int8_t filter = sector->filter;
    int32_t f0 = pos_adpcm_table[filter];
    int32_t f1 = neg_adpcm_table[filter];

    /* shift 13-15 treated as 9 */
    shift = (shift > 12) ? 3: 12 - shift;

    /* previous samples */
    int16_t _old = *old, _older = *older;
    for (uint32_t s = 0; s < 14; s++) {
        int32_t lsb, msb;

        /* get each four bit nibble */
        lsb = ((sector->data[s]) >> 0) & 0xf;
        msb = ((sector->data[s]) >> 4) & 0xf;
        
        int8_t signbit = 1 << 3;

        lsb = signextend(lsb, signbit);
        msb = signextend(msb, signbit);

        /* compute the first sample */
        lsb = (lsb << shift) + (((_old * f0) + (_older * f1) + 32)/64);
        lsb = CLAMP(lsb, 0x7fff, -0x8000);

        /* store the first sample */
        decode_buffer[2*s+0] = lsb;

        /* update the sample history */
        _older = _old;
        _old   = lsb;

        /* compute the second sample */
        msb = (msb << shift) + (((_old * f0) + (_older * f1) + 32)/64);
        msb = CLAMP(msb, 0x7fff, -0x8000);

        /* store the second sample */
        decode_buffer[2*s+1] = msb;

        /* update the sample history */
        _older = _old;
        _old   = msb;
    }

    *older = _older;
    *old   = _old;
}

static inline void spu_decode_block(int32_t v) {
    struct spu_adpcm_sector *sector;
    struct spu_voice        *voice;

    /* retrive voice */
    voice = &spu.voices[v];

    /* get sector pointer */
    memory_read_sram_sector(voice->current_address, &sector);

    /* populate voice samples buffer */
    spu_decode_samples(sector, voice->decoded, 
                       &voice->old, &voice->older);

    /* sets the repeat address if current block has loop start set */
    if (sector->loop_start) {
        voice->repeat_address = 
            voice->current_address >> 3;
    }

    /* jumps to adpcm repeat address if current block has loop end set */
    if (sector->loop_end) {
        voice->current_address =
            voice->repeat_address << 3;

        /* silence voice (volume 0) */
        if (!sector->loop_repeat) {
            spu.endx |= (1 << v);
            spu.koff |= (1 << v); // BUG: seems like adsr related
        }
    } else {
        /* increment current address otherwise */
        voice->current_address += 16;
    }
}

static inline int32_t spu_service_voice(uint32_t v) {
    /* retrive the voice */
    struct spu_voice *voice = &spu.voices[v];

    // TRACE_VOICE;

    /* check for keyon */
    if (spu.kon & (1 << v)) {
        /* copy start address to current address */
        voice->current_address = 
            voice->start_address << 3;

        /* reset pitch counter */
        voice->pitch_counter = 0;

        /* reset decode buffer index */
        voice->index = 0;

        voice->old   = 0;
        voice->older = 0;

        /* pre-fill the buffer */
        spu_decode_block(v);

        /* clear keyon and endx flag */
        spu.endx &= ~(1 << v);
        spu.kon  &= ~(1 << v); // BUG: seems adsr related
    }

    int16_t step = voice->sample_rate;
    /* clamp and add the sample rate to the pitch counter */
    voice->pitch_counter += (step < 0x4000) ? step: 0x4000;

    /* pitch counter determines how many samples *
     * are stepped through during each spu clock */
    while(voice->pitch_counter > 0x1000) {
        /* decrement the pitch counter */
        voice->pitch_counter -= 0x1000;
        /* increment the sample index */
        voice->index++;

        /* if the decode buffer index reaches the end, *
         * decode new samples and  set index to 0      */
        if (voice->index == 28) {
            voice->index = 0;
            spu_decode_block(v);
        }
    }

    return voice->decoded[voice->index];

    /* add adsr handling */

    /* add volume and panning */
}

#define MIX_SHIFT 1
static inline int16_t spu_mix_samples(int32_t mix) {
    int32_t sample = mix >> MIX_SHIFT;

    sample = (sample < INT16_MIN) ? INT16_MIN: sample;
    sample = (sample > INT16_MAX) ? INT16_MAX: sample;

    return sample;
}

void init_spu(void) {
    spu = (struct spu) {};
}
void task_spu( void ) {
    int32_t acc = 0;

    for (uint32_t v = 0; v < 24; v++) {
        acc += spu_service_voice(v) >> 4;
    }

    if (acc >INT16_MAX || acc < INT16_MIN)
        printf("CLIP: %d", acc);

    int16_t current_sample = 
        spu_mix_samples(acc);

    system_audio_push_sample(current_sample, 
                             current_sample);
}
