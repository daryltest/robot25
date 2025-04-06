// C library headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// Linux headers
#include <fcntl.h>                 // Contains file controls like O_RDWR
#include <errno.h>                 // Error integer and strerror() function
#include <termios.h>               // Contains POSIX terminal control definitions
#include <unistd.h>                // write(), read(), close()
#include <stdint.h>

#define UART_RVC_PATH "/dev/ttyS0" // Or /dev/ttyS2 if P9.22 was used
#define UART_RVC_PACKET_BYTES 19

int uart_fd;
void poll();

int main(void)
{
    struct termios tio;
    uart_fd = open(UART_RVC_PATH, O_RDWR);
    if (uart_fd == -1)
    {
        perror("Error opening UART");
        exit(-1);
    }
    tcgetattr(uart_fd, &tio);
    cfsetispeed(&tio, B115200); // Set baud rate to 115200
    cfsetospeed(&tio, B115200); // Set baud rate to 115200
    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag |= CREAD | CLOCAL;
    tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);
    tio.c_oflag &= ~OPOST;
    tio.c_oflag &= ~ONLCR;
    tio.c_iflag &= ~(IXON | IXOFF | IXANY);
    tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tio.c_cc[VTIME] = 0;
    tio.c_cc[VMIN] = 0;
    //tio.c_cc[VMIN] = UART_RVC_PACKET_BYTES; // 19 bytes
    tio.c_cflag &= ~CSIZE;              // clear bit data
    tio.c_cflag |= CS8;                 // 8-bit data
    tio.c_cflag &= ~PARENB;             // No parity
    tio.c_cflag &= ~CSTOPB;             // 1 stop bit
    tcsetattr(uart_fd, TCSANOW, &tio);
    // close(uart_fd); // Closing the connection

    poll();

    return 0;
}

void poll() {
    while (1)
    {
        uint8_t fd;
        unsigned char buffer[100];
        memset(&buffer, '\0', sizeof(buffer));
        ssize_t bytes_read = read(uart_fd, buffer, sizeof(buffer));
        int found_header = 0;
        int i = 0;
        int rnd = 0;

        if (bytes_read > 0) {
            // for (i = 1; i < bytes_read && !found_header; ++i) {
            //     found_header = (buffer[i] == 0xAA && buffer[i - 1] == 0xAA);

            //     // if (buffer[i] == 0xAA && buffer[i - 1] == 0xAA) {
            //     //     found_header = 1;
            //     // }
            // }

            // if (found_header) {
            //     printf("%3d : ", i);

            //     if (i == 2)
            //     {
            //         uint8_t seq = buffer[2];
            //         printf("%3i : ", (int)seq);

            //         uint16_t yaw_raw = (buffer[4] << 8) | buffer[3]; // Byte 8,9: Roll LSB, MSB
            //         int16_t yaw = (int16_t) yaw_raw;
            //         double value = (yaw * .01); // Converts int to float
    
            //         //printf("%d       ", yaw);
            //         printf("%8.2f°      ", value);
            //     } else {
            //         //printf("       : ");
            //     }
            // } else {
            //     //printf("    :       : ", i);
            // }

            printf("%3d: ", bytes_read);

            for (i = 0; i < bytes_read; ++i) {
                // printf("%02X ", (int) (buffer[i]));
            }

            printf("\n");
        }


        usleep(35000); // 10 ms for 100Hz
    }
}