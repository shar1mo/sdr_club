#include <SoapySDR/Device.h> 
#include <SoapySDR/Formats.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <complex.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

typedef struct {
    double K1, K2;
    double p1, p2;
    int offset;
    int N_sps;  // samples per symbol
} SymbolSync;

void init_symbol_sync(SymbolSync *sync, int samples_per_symbol) {
    sync->N_sps = 10;
    sync->p1 = 0.0;
    sync->p2 = 0.0;
    sync->offset = 0;
    
    double loop_bandwidth = 0.01; //BnTs
    double damping_factor = sqrt(2)/2; //zeta
    double detector_gain = 0.0002; //Kp
    
    double theta = (loop_bandwidth / samples_per_symbol) / (damping_factor + 1.0 / (4.0 * damping_factor));
    double denominator = 1.0 + 2.0 * damping_factor * theta + theta * theta;
    
    sync->K1 = (4.0 * damping_factor * theta) / (denominator * detector_gain);
    sync->K2 = (4.0 * theta * theta) / (denominator * detector_gain);
    
    printf("Symbol sync: K1=%f, K2=%f, N_sps=%d\n", sync->K1, sync->K2, sync->N_sps);
}

double gardner_ted(const int16_t *samples, int n, int N_sps) {
    int N_sp = N_sps / 2;
    
    int idx_mid = n + N_sp;
    int idx_prev = n;
    int idx_next = n + N_sps;
    
    double x_mid = samples[idx_mid];
    double x_prev = samples[idx_prev];
    double x_next = samples[idx_next];
    
    double scale = 1000.0;
    double error = (x_mid / scale) * ((x_next - x_prev) / scale);
    
    if (error > 1.0) error = 1.0;
    if (error < -1.0) error = -1.0;
    
    return error;
    return x_mid * (x_next - x_prev);
}

void symbol_synchronization(const int16_t *real_samples, int num_samples) {
    SymbolSync sync;
    int samples_per_symbol = 10;
    init_symbol_sync(&sync, samples_per_symbol);
    
    int start_index = 0;
    for(int i = 0; i < num_samples - 100; i++) {
        if(abs(real_samples[i]) > 1000) {
            start_index = i;
            break;
        }
    }
    printf("Signal starts at index: %d\n", start_index);
    
    FILE *sync_file = fopen("../symbol_synchronized.pcm", "wb");
    if (!sync_file) {
        printf("Error creating sync file\n");
        return;
    }
    
    int symbol_count = 0;
    int max_symbols = (num_samples - start_index) / samples_per_symbol;
    
    printf("Starting symbol synchronization...\n");
    

    double final_error = 0.0;
    int final_offset = 0;
    double final_p2 = 0.0;
    
    for(int symbol_idx = 0; symbol_idx < max_symbols; symbol_idx++) {
        int base_index = symbol_idx * samples_per_symbol + start_index;
        
        if(base_index + sync.N_sps >= num_samples) {
            break;
        }
        
        double error = gardner_ted(real_samples, base_index, sync.N_sps);
        
        sync.p1 = error * sync.K1;
        sync.p2 = sync.p2 + sync.p1 + error * sync.K2;
        
        while(sync.p2 >= 1.0) sync.p2 -= 1.0;
        while(sync.p2 < 0.0) sync.p2 += 1.0;
        
        sync.offset = (int)round(sync.p2 * sync.N_sps);
        
        int optimal_index = base_index + sync.offset;
        
        if(optimal_index < num_samples) {
            int16_t symbol_value = real_samples[optimal_index];
            fwrite(&symbol_value, sizeof(int16_t), 1, sync_file);
            symbol_count++;
        }
        
        final_error = error;
        final_p2 = sync.p2;
        final_offset = sync.offset;
        
        if(symbol_idx < 15) {
            printf("Symbol %d: error=%f, p2=%f, offset=%d\n", 
                   symbol_idx, error, sync.p2, sync.offset);
        }
    }
    
    fclose(sync_file);
    printf("Symbol synchronization completed: %d symbols saved\n", symbol_count);
    
    printf("Final synchronization values:\n");
    printf("  Final error: %f\n", final_error);
    printf("  Final p2: %f\n", final_p2);
    printf("  Final offset: %d\n", final_offset);
    printf("  Final loop filter state: p1=%f, p2=%f\n", sync.p1, sync.p2);
}

void read_and_sync_real_part() {
    FILE *file = fopen("../real_part_filtered.pcm", "rb");
    if(!file) {
        printf("Error opening real_part_filtered.pcm\n");
        return;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    int num_samples = file_size / sizeof(int16_t);
    int16_t *real_samples = (int16_t*)malloc(file_size);
    
    if(fread(real_samples, 1, file_size, file) != file_size) {
        printf("Error reading file\n");
        fclose(file);
        free(real_samples);
        return;
    }
    
    fclose(file);
    printf("Read %d samples from real_part_filtered.pcm\n", num_samples);
    
    symbol_synchronization(real_samples, num_samples);
    
    free(real_samples);
}

int *to_bpsk(int *bit_arr, int length) {
    int *bpsk_arr = (int *)malloc(length * sizeof(int));
    for(int i = 0; i < length; i++) {
        if(bit_arr[i] == 0) {
            bpsk_arr[i] = 1;
        } else {
            bpsk_arr[i] = -1;
        }
    }
    return bpsk_arr;
}

int *upsampling(int *bpsk_arr, int length) {
    int count = 0;
    int *bpsk_after_upsampling = (int *)malloc(length * 10 * sizeof(int));
    
    for(int i = 0; i < length; i++) {
        if (i > 0) {
            for(int j = 0; j < 9; j++) {
                bpsk_after_upsampling[count] = 0;
                count++;
            }
        }
        bpsk_after_upsampling[count] = bpsk_arr[i];
        count++;
    }
    return bpsk_after_upsampling;
}

int *convolution(int *upsampling_arr, int *impulse_arr, int length, int impulse_length) {
    int result_length = length;
    int *upsampl_after_conv = (int *)malloc(result_length * sizeof(int));
    
    for (int i = 0; i < result_length; i++) {
        upsampl_after_conv[i] = 0;
    }
    
    for (int n = 0; n < result_length; n++) {
        for (int k = 0; k < impulse_length; k++) {
            if (n - k >= 0 && n - k < length) {
                upsampl_after_conv[n] += upsampling_arr[n - k] * impulse_arr[k];
            }
        }
    }
    
    return upsampl_after_conv;
}

int main(void) {

    printf("Symbol Synchronization\n");
    read_and_sync_real_part();
    printf("\n");
    
    printf("BPSK Transmission\n");
    int bit_arr[] = {0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,1,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,0,0,1,1,0,0,0,1,1,0,1,1,0,1,1,0,0,0,1,1,1,0,1,0,1,0,1,1,0,0,0,1,0}; //hellosdrclub
    int len_arr = sizeof(bit_arr) / sizeof(bit_arr[0]);
    printf("Length of bit array: %d\n", len_arr);
    
    int *bpsk_arr = to_bpsk(bit_arr, len_arr);
    printf("BPSK array: ");
    for(int i = 0; i < len_arr; i++) {
    printf("%d ", bpsk_arr[i]);
    }
    printf("\n");

    int *bpsk_after_arr = upsampling(bpsk_arr, len_arr);
    printf("After upsampling: ");
    for(int i = 0; i < len_arr * 10; i++) {
    printf("%d ", bpsk_after_arr[i]);
    }
    printf("\n");

    int pulse[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    int pulse_length = 10;

    int *conv_result = convolution(bpsk_after_arr, pulse, len_arr * 10, pulse_length);

    printf("After convolution: ");
    int conv_length = len_arr * 10;
    for(int i = 0; i < conv_length; i++) {
    printf("%d ", conv_result[i]);
    }
    printf("\n");


    int16_t *tx_samples = (int16_t *)malloc(conv_length * 2 * sizeof(int16_t));
    int scale_factor = 1;
    
    for(int i = 0; i < conv_length; i++) {
        tx_samples[i * 2] = (int16_t)(conv_result[i] * scale_factor);
        tx_samples[i * 2 + 1] = 0;
    }
    
    printf("Converted to %d I/Q samples\n", len_arr * 10);

    FILE *fpt_pcm = fopen("../symb_before_send.pcm", "wb");
    if (fpt_pcm) {
        fwrite(tx_samples, sizeof(int16_t), len_arr * 10 * 2, fpt_pcm);
        fclose(fpt_pcm);
        printf("Saved samples to symb_before_send.pcm\n");
    }

    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr"); 
    if (1) {
        SoapySDRKwargs_set(&args, "uri", "usb:"); 
    } else {
        SoapySDRKwargs_set(&args, "uri", "ip:192.168.2.1");
    }
    SoapySDRKwargs_set(&args, "direct", "1");
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL) {
        printf("Failed to create SDR device\n");
        return EXIT_FAILURE;
    }

    int sample_rate = 1e6;
    int carrier_freq = 800e6;

    // Параметры RX части
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, carrier_freq, NULL);

    // Параметры TX части
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, carrier_freq, NULL);

    // Инициализация количества каналов
    size_t channels[] = {0};
    
    // Настройки усилителей
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels, 25.0);
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels, -30.0);

    const size_t channel_count = 1;
    
    // Формирование потоков
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);

    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0);
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0);

    // Получение MTU
    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);

    printf("RX MTU: %zu, TX MTU: %zu\n", rx_mtu, tx_mtu);

    int16_t *tx_buff = (int16_t*)malloc(2 * tx_mtu * sizeof(int16_t));
    int16_t *rx_buffer = (int16_t*)malloc(2 * rx_mtu * sizeof(int16_t));

    // Файл для записи принятых данных
    FILE *fptr = fopen("../symb_after_rx.pcm", "wb");
    if(fptr == NULL) {
        perror("fptr err");
        return EXIT_FAILURE;
    }

    const long timeoutUs = 400000;

    // Передача подготовленных сэмплов
    int total_samples_sent = 0;
    int flags = SOAPY_SDR_HAS_TIME;
    long long tx_time = 0;

    printf("Starting transmission...\n");

    // Сначала получаем временную метку для синхронизации
    void *rx_buffs[] = {rx_buffer};
    int rx_flags;
    long long timeNs;
    int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &rx_flags, &timeNs, timeoutUs);
    if (sr > 0) {
        tx_time = timeNs + (4 * 1000 * 1000); // на 4 мс в будущее
        printf("Got timestamp for sync: %lld\n", timeNs);
    }

    while (total_samples_sent < conv_length) {
        int samples_to_send = (conv_length - total_samples_sent < tx_mtu) ? 
                             (conv_length - total_samples_sent) : tx_mtu;

        // Копируем сэмплы в tx_buff
        for (int i = 0; i < samples_to_send * 2; i++) {
            tx_buff[i] = tx_samples[total_samples_sent * 2 + i] * 1500 << 4;
        }

        for (int i = samples_to_send * 2; i < tx_mtu * 2; i++) {
            tx_buff[i] = 0;
        }

        void *tx_buffs[] = {tx_buff};
        int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_mtu, &flags, tx_time, timeoutUs);
        
        if (st < 0) {
            printf("TX Failed: %i\n", st);
            break;
        }
        
        total_samples_sent += samples_to_send;
        tx_time += (samples_to_send * 1000000000LL) / sample_rate;
        
        printf("Sent %d samples, total: %d/%d\n", samples_to_send, total_samples_sent, conv_length);
    }

    printf("Transmission completed. Total samples sent: %d\n", total_samples_sent);

    printf("Starting reception...\n");
    size_t iteration_count = 10;
    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++) {
        void *rx_buffs[] = {rx_buffer};
        int flags;
        long long timeNs;
        
        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);

        if (sr > 0) {
            fwrite(rx_buffer, sr * 2 * sizeof(int16_t), 1, fptr);
            printf("Received buffer %lu: %d samples\n", buffers_read, sr);
        } else {
            printf("RX Failed: %i\n", sr);
        }
    }

    printf("Reception completed\n");

    fclose(fptr);
    free(tx_buff);
    free(rx_buffer);
    free(tx_samples);
    free(bpsk_arr);
    free(bpsk_after_arr);
    free(conv_result);

    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);
    SoapySDRDevice_unmake(sdr);

    return EXIT_SUCCESS;
}