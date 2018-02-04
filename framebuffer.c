/*
To test that the Linux framebuffer is set up correctly, and that the device permissions
are correct, use the program below which opens the frame buffer and draws a gradient-
filled red square:
retrieved from:
Testing the Linux Framebuffer for Qtopia Core (qt4-x11-4.2.2)
http://cep.xor.aps.anl.gov/software/qt4-x11-4.2.2/qtopiacore-testingframebuffer.html
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#define maxY 750
#define maxX 1350

struct Point{
    int y;
    int x;
};

void startBuffer(char*** buffer){
    (*buffer) = (char**) malloc (maxY * sizeof(char*));
    for (int i = 1; i < maxY; i++)
        (*buffer)[i] = (char*) malloc (maxX * sizeof(char*));
}

void setBlackScreen(char*** buffer){
    for (int i=1; i < maxY; i++)
        for (int j=1; j < maxX; j++)
            (*buffer)[i][j] = '-';
}

void drawVerticalLine(char*** buffer, int start, int finish, int horizontalPosition){
    if (finish < start){
        int temp = start;
        start = finish;
        finish = temp;
    }
    for (int i=start; i<= finish; i++)
        (*buffer)[i][horizontalPosition] = '#';
}

void drawHorizontalLine(char*** buffer, int start, int finish, int verticalPosition){
    if (finish < start){
        int temp = start;
        start = finish;
        finish = temp;
    }
    for (int i=start; i<= finish; i++)
        (*buffer)[verticalPosition][i] = '#';
}

void drawC(char*** buffer, struct Point A, struct Point B, struct Point C, struct Point D, struct Point E, struct Point F, struct Point G, struct Point H){
    drawVerticalLine(buffer, A.y, B.y, A.x);
    drawHorizontalLine(buffer, B.x, C.x, B.y);
    drawVerticalLine(buffer, C.y, D.y, C.x);
    drawHorizontalLine(buffer, D.x, E.x, E.y);
    drawVerticalLine(buffer, E.y, F.y, E.x);
    drawHorizontalLine(buffer, F.x, G.x, F.y);
    drawVerticalLine(buffer, G.y, H.y, G.x);
    drawHorizontalLine(buffer, H.x, A.x, H.y);
}


void draw(struct fb_var_screeninfo vinfo, struct fb_fix_screeninfo finfo, char *fbp, char ***buffer, long int location ){

    for (int y = 1; y < maxY; y++)
        for (int x = 1; x < maxX; x++) {

            location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y+vinfo.yoffset) * finfo.line_length;

            if (vinfo.bits_per_pixel == 32) {
                if ((*buffer)[y][x] == '-') {
                    *(fbp + location) = 0;        // Some blue
                    *(fbp + location + 1) = 0;     // A little green
                    *(fbp + location + 2) = 0;    // A lot of red
                    *(fbp + location + 3) = 0;      // No transparency
                } else if ((*buffer)[y][x] == '#'){
                    *(fbp + location) = 255;        // Some blue
                    *(fbp + location + 1) = 255;     // A little green
                    *(fbp + location + 2) = 255;    // A lot of red
                    *(fbp + location + 3) = 255;      // No transparency

                }
        //location += 4;
            } else  { //assume 16bpp
                if ((*buffer)[y][x] == '-') {
                    int b = 0;
                    int g = 0;     // A little green
                    int r = 0;    // A lot of red
                    unsigned short int t = r<<11 | g << 5 | b;
                    *((unsigned short int*)(fbp + location)) = t;
                } else if ((*buffer)[y][x] == '#') {
                    int b = 255;
                    int g = 255;     // A little green
                    int r = 255;    // A lot of red
                    unsigned short int t = r<<11 | g << 5 | b;
                    *((unsigned short int*)(fbp + location)) = t;
                }
            }

        }
}

int main()
{
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int x = 0, y = 0;
    long int location = 0;
    char **buffer;
    startBuffer(&buffer);

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

    x = 1; y = 1;       // Where we are going to put the pixel


    setBlackScreen(&buffer);

    struct Point A, B, C, D, E, F, G, H;
    A.x = 100; A.y = 100; B.x = 100; B.y = 250; C.x = 200; C.y = 250; D.x = 200; D.y = 225; E.x = 125; E.y = 225; F.x = 125; F.y = 125; G.x = 200; G.y = 125; H.x = 200; H.y = 100;
    drawC(&buffer, A, B, C, D, E, F, G, H);    
    //Kotak yang diisi minY = 75 maxY = 275 minX = 75 maxX = 225 (untuk Huruf C)

    int minKotakY = 100, maxKotakY = 275, minKotakX = 75, maxKotakX = 225; 
    int iteranY = minKotakY, iteranX = minKotakX;    bool fill = false; 
    while(1) {
        if (iteranY < maxKotakY){
            if ((buffer[iteranY][iteranX] == '-') && (buffer[iteranY][iteranX+1] == '#') && (buffer[iteranY][iteranX+2] == '-')){
                if (fill) {
                    fill = false;
                }
                else {
                    fill = true;
                    iteranX++;
                }
            } 
            if (iteranX == maxKotakX){
                iteranY ++; 
                iteranX = minKotakX;
                fill = false;
            }

            if (fill)
                 buffer[iteranY][iteranX] = '#';
            else 
                 buffer[iteranY][iteranX] = '-';

        }
        draw(vinfo, finfo, fbp, &buffer, location);
        iteranX++;
    }


    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}