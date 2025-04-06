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
#include <sys/ioctl.h>

#define UART_RVC_PATH "/dev/ttyS0" // Or /dev/ttyS2 if P9.22 was used
#define UART_RVC_PACKET_BYTES 19

static int gyro_uart_fd;
int32_t poll_gyro();
void setup_gyro();
static unsigned char buffer[150];
static int partial_response_bytes = 0;

int main(void) {
    setup_gyro();

    while (1) {
        int yaw = poll_gyro();

        if (yaw != INT32_MIN) {
            double value = (yaw * .01); // Converts int to float
            printf("%8.2f°\n", value);
        }

        usleep(2500); // "other work"
    }
    return 0;
}

void setup_gyro() {
    gyro_uart_fd = open(UART_RVC_PATH, O_RDWR);
    if (gyro_uart_fd == -1)
    {
        perror("Error opening UART");
        exit(-1);
    }

    struct termios tio;
    tcgetattr(gyro_uart_fd, &tio);

    cfsetispeed(&tio, B115200); // Set baud rate to 115200
    cfsetospeed(&tio, B115200); // Set baud rate to 115200

    tio.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    tio.c_cflag |= CREAD | CLOCAL | CS8;

    tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);
    tio.c_oflag &= ~(OPOST | ONLCR);
    tio.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    tio.c_cc[VTIME] = 0;
    tio.c_cc[VMIN] = 0;

    tcsetattr(gyro_uart_fd, TCSANOW, &tio);
}

// Returns integer yaw as .01 deg. Values are -17999 -> +18000.
// Returns INT32_MIN if no new data is available. Doesn't block.

int32_t poll_gyro() {
    int32_t gyro_yaw;

    int bytes_waiting;
    ioctl(gyro_uart_fd, TIOCINQ, &bytes_waiting);
    if (bytes_waiting == 0) {
        return INT32_MIN;
    }

    // Read however many bytes are available

    ssize_t bytes_read = read(gyro_uart_fd, buffer + partial_response_bytes, sizeof(buffer) - partial_response_bytes);
    partial_response_bytes += bytes_read;
    if (partial_response_bytes < UART_RVC_PACKET_BYTES) {
        return INT32_MIN;
    }

    // Check for a complete, valid packet. Start with the largest index which could possibly fit a complete 19-byte
    // message, and work backwards.

    unsigned char *complete_packet;
    int found_complete = 0;

    for (complete_packet = buffer + partial_response_bytes - UART_RVC_PACKET_BYTES;
        complete_packet >= buffer && !found_complete;
        --complete_packet
    ) {
        if (complete_packet[0] == 0xAA && complete_packet[1] == 0xAA) {
            found_complete = 1;
            break;
        }
    }

    if (found_complete) {
        uint8_t sequence_num = complete_packet[2];
        int16_t yaw_16 = complete_packet[3] | (complete_packet[4] << 8); // 3, 4: Yaw LSB, MSB
        gyro_yaw = (int32_t) yaw_16;
    }

    // The last 18 bytes could plausibly be the start of a partially-received packet. A partial packet consists of
    // (0xAA,0xAA) and everything after it, or the special case of a single 0xAA at the very last position in the buffer.

    int i;
    if (found_complete) {
        i = (complete_packet - buffer) + 19;
    } else {
        i = partial_response_bytes - UART_RVC_PACKET_BYTES + 1;
    }
    int found_partial = 0;

    for (; i < partial_response_bytes; ++i) {
        if (
            buffer[i] == 0xAA &&
            (i + 1 == partial_response_bytes || buffer[i + 1] == 0xAA)
        ) {
            found_partial = 1;
            break;
        }
    }

    if (found_partial) {
        partial_response_bytes -= i;
        memmove(buffer + i, buffer, partial_response_bytes);
    } else {
        partial_response_bytes = 0;
    }

    return (int32_t) gyro_yaw;
}
