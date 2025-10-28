#include <SoapySDR/Device.h> 
#include <SoapySDR/Formats.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <complex.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>


#include <stdio.h>
#include <stdlib.h>

int *to_bpsk(int *bit_arr, int length) {
    int *bpsk_arr = (int *)malloc(length * 2 * sizeof(int)); 
    for(int i = 0; i < length; i++) {
        if(bit_arr[i] == 0) {
            bpsk_arr[i * 2] = 1;
        } else {
            bpsk_arr[i * 2] = -1;
        }
        bpsk_arr[i * 2 + 1] = 0;
    }
    return bpsk_arr;
}

int *upsampling(int *bpsk_arr, int length) {
    int count = 0; 
    int *bpsk_after_upsampling = (int *)malloc(length * 2 * 10 * sizeof(int));
    
    for(int i = 0; i < length * 2; i++) {
        bpsk_after_upsampling[count] = bpsk_arr[i];
        count++;
        for(int j = 0; j < 9; j++) {
            bpsk_after_upsampling[count] = 0;
            count++;
        }
    }
    return bpsk_after_upsampling;
} 

int main(void) {
    int bit_arr[] = {0,1,1,0,1,0,0,0,0,1,1,0,0,1,0,1,0,1,1,0,1,1,0,0,0,1,1,0,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,0,1,1,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,0,0,1,1,0,0,0,1,1,0,1,1,0,1,1,0,0,0,1,1,1,0,1,0,1,0,1,1,0,0,0,1,0};
    int len_arr = sizeof(bit_arr) / sizeof(bit_arr[0]);
    printf("%d\n", len_arr);
    
    int *bpsk_arr = to_bpsk(bit_arr, len_arr);
    for(int i = 0; i < len_arr * 2; i++) {
        printf("%d ", bpsk_arr[i]);
    }
    printf("\n");
    
    int *bpsk_after_arr = upsampling(bpsk_arr, len_arr);
    for(int i = 0; i < len_arr * 2 * 10; i++) {
        printf("%d ", bpsk_after_arr[i]);
    }
    
    free(bpsk_arr);
    free(bpsk_after_arr);
    return 0;
}
  

//   SoapySDRKwargs args = {};
//   SoapySDRKwargs_set(&args, "driver", "plutosdr"); 
//   if (1) {
//     SoapySDRKwargs_set(&args, "uri", "usb:"); 
//   } else {
//     SoapySDRKwargs_set(&args, "uri", "ip:192.168.2.1");
//   }
//   SoapySDRKwargs_set(&args, "direct", "1");
//   SoapySDRKwargs_set(&args, "timestamp_every", "1920");   // Размер буфера + временные метки
//   SoapySDRKwargs_set(&args, "loopback", "0");             // Используем антенны или нет
//   SoapySDRDevice *sdr = SoapySDRDevice_make(&args);       // Инициализация
//   SoapySDRKwargs_clear(&args);

//   int sample_rate = 1e6;
//   int carrier_freq = 800e6;
//   // Параметры RX части
//   SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate);
//   SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, carrier_freq , NULL);

//   // Параметры TX части
//   SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate);
//   SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, carrier_freq , NULL);

//   // Инициализация количества каналов RX\\\\TX (в AdalmPluto он один, нулевой)
//   size_t channels[] = {0};
//   // Настройки усилителей на RX\\\\TX
//   SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels, 25.0);
//   SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels, -30.0);

//   const size_t channel_count = 1;
//   // Формирование потоков для передачи и приема сэмплов
//   SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
//   SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);

//   SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0); //start streaming
//   SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0); //start streaming

//   // Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
//   size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
//   size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);

//   int16_t tx_buff[2*tx_mtu];
//   int16_t rx_buffer[2*rx_mtu];

//   FILE *fpt_pcm;
//   fpt_pcm = fopen("../symb_befor_send.pcm", "wb");
//   fseek(fpt_pcm, 0, SEEK_END);
//   long file_pcm_size = ftell(fpt_pcm);
//   fseek(fpt_pcm, 0, SEEK_SET);
//   printf("smeshariki_pcm_size = %ld\n", file_pcm_size);
//   int16_t *samples_pcm = (int16_t *)malloc(file_pcm_size);
//   size_t sample_count = file_pcm_size / sizeof(int16_t);
//   size_t sf = fread(samples_pcm, sizeof(int16_t), sample_count, fpt_pcm);

//   if(sf == 0){
//     perror("file is empty");
//   }

//   fclose(fpt_pcm);
  

//   //заполнение tx_buff значениями сэмплов первые 16 бит - I, вторые 16 бит - Q.
//   // for (int i = 0; i < 2 * tx_mtu; i+=2) {
//   //   // ЗДЕСЬ БУДУТ ВАШИ СЭМПЛЫ
//   //   tx_buff[i] = 400 << 4;   // I
//   //   tx_buff[i+1] = 400 << 4; // Q
//   // }

//   // Количество итерация чтения из буфера
//   size_t iteration_count = 844;
//   const long  timeoutUs = 400000;
//   long long last_time = 0;


//   FILE *fptr;
//   char *file_name = "../symb_after_rx.pcm";

//   fptr = fopen(file_name, "wb");
//   if(fptr == NULL) {
//     perror("fptr err");
//     return EXIT_FAILURE;
//   }

// // Начинается работа с получением и отправкой сэмплов
//   for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++)
//   {
//     void *rx_buffs[] = {rx_buffer};
//     int flags;        // flags set by receive operation
//     long long timeNs; //timestamp for receive buffer
    
//     // считали буффер RX, записали его в rx_buffer
//     int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);


//     fwrite(rx_buffer, 2 * rx_mtu * sizeof(int16_t), 1, fptr);

//     // Смотрим на количество считаных сэмплов, времени прихода и разницы во времени с чтением прошлого буфера
//     printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", buffers_read, sr, flags, timeNs, timeNs - last_time);
//     last_time = timeNs;

//     // Переменная для времени отправки сэмплов относительно текущего приема
//     long long tx_time = timeNs + (4 * 1000 * 1000); // на 4 [мс] в будущее

//     // Добавляем время, когда нужно передать блок tx_buff, через tx_time -наносекунд
//       for(int i = 0; i < 2 * tx_mtu; i+=2){
//         tx_buff[i] = samples_pcm[i + 1920 * buffers_read];
//         tx_buff[i + 1] = samples_pcm[i + 1 + 1920 * buffers_read];
//       }
//       for(size_t i = 0; i < 2; i++) {
//         tx_buff[0 + i] = 0xffff;
//         tx_buff[10 + i] = 0xffff;
//       }
//       for(size_t i = 0; i < 8; i++) {
//           uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;
//           tx_buff[2 + i] = tx_time_byte << 4;
//       }  

//     // Здесь отправляем наш tx_buff массив
//     void *tx_buffs[] = {tx_buff};
//       printf("buffers_read: %d\n", buffers_read);
//       flags = SOAPY_SDR_HAS_TIME;

//       int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_mtu, &flags, tx_time, timeoutUs);
//       if ((size_t)st != tx_mtu) {
//         printf("TX Failed: %i\n", st);
//       }
//   }
//   printf("write done\n");
//   fclose(fptr);


//   //stop streaming
//   SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
//   SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);

//   //shutdown the stream
//   SoapySDRDevice_closeStream(sdr, rxStream);
//   SoapySDRDevice_closeStream(sdr, txStream);

//   //cleanup device handle
//   SoapySDRDevice_unmake(sdr);

//   return EXIT_SUCCESS;
// }