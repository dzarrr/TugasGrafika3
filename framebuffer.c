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

char *fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

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

void drawLine(char*** buffer, struct Point p1, struct Point p2)
{
    int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i, x1, x2, y1, y2;
    x1 = p1.x;
    x2 = p2.x;
    y1 = p1.y;
    y2 = p2.y;
    dx = p2.x - p1.x;
    dy = p2.y - p1.y;
    dx1 = abs(dx);
    dy1 = abs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;
    if (dy1 <= dx1)
    {
        if (dx>=0)
        {
            x = x1;
            y = y1;
            xe = x2;
        }
        else
        {
            x = x2;
            y = y2;
            xe = x1;
        }
        (*buffer)[y][x] = '#';
        for (i = 0; x < xe; i++)
        {
            x = x + 1;
            if (px < 0)
            {
                px = px + 2 * dy1;
            }
            else
            {
                if((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                {
                    y = y + 1;
                }
                else
                {
                    y = y - 1;
                }
                px = px + 2 * (dy1 - dx1);
            }       
            (*buffer)[y][x] = '#';
        }     
    }
    else
    {
        if ( dy >= 0 )
        {
            x = x1;
            y = y1;
            ye = y2;
        }
        else
        {
           x = x2;
           y = y2;
           ye = y1;
        }
        (*buffer)[y][x] = '#';
        for (i = 0; y < ye; i++)
        {
            y = y + 1;
            if (py <= 0)
            {
                py= py + 2 * dx1;
            }
            else
            {
                if (( dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                {
                    x = x + 1;
                }
                else
                {
                    x = x - 1;
                }
                py = py + 2 * (dx1 - dy1);
            }
            (*buffer)[y][x] = '#';
        }
    }
}


void draw(char*** buffer){
    long int location;
    for (int y = 1; y < maxY; y++) {
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
}

void raster(char*** buffer, int i, int minKotakX, int maxKotakX) {
    struct Point* pos = (struct Point*) malloc (100 * sizeof(struct Point*));
    int count = 0;
    int j = minKotakX;

    while (j <= maxKotakX) {
        if ((*buffer)[i][j] == '#') {
            pos[count].x = j;
            pos[count].y = i;
            count++;
        }
        j++;
    }
    if (count > 1) {
        if (count % 2 == 0) {
            bool fill = true;
            int now = 1;
            while (now < count) {
                if (fill)
                    for (int k = pos[now-1].x; k < pos[now].x; k++)
                        (*buffer)[pos[now].y][k] = '#';
                now++;
                fill = !fill;
            }
        }
        else {
            bool fill = true;
            int now = 1;
            while (now < count) {
                if (fill)
                    for (int k = pos[now-1].x; k < pos[now].x; k++)
                        (*buffer)[pos[now].y][k] = '#';
                if (now != count / 2)
                    fill = !fill;
                now++;
            }
        }
    }
}

void drawSquare(char*** buffer, int size, int xOrigin, int yOrigin){
    struct Point A, B, C, D;
    A.x = xOrigin - size; A.y = yOrigin - size;
    B.x = xOrigin + size; B.y = yOrigin - size;
    C.x = xOrigin + size; C.y = yOrigin + size;
    D.x = xOrigin - size; D.y = yOrigin + size;

    drawLine(buffer, A, B); drawLine(buffer, B, C); drawLine(buffer, C, D); drawLine(buffer, D, A);
    for (int i = 5; i <= yOrigin + size; i++){
        raster(buffer, i, 5, 1300);
    }
}

int main()
{
    int fbfd = 0;
    long int screensize = 0;
    int x = 0, y = 0;
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

    setBlackScreen(&buffer);

    int minKotakY = 100, maxKotakY = 275;   
    bool fill = false; 
    bool edge = false; 
    bool first = false;
    int minKotakX, maxKotakX, j;

    int xOrigin = 675; 
    int yOrigin = 200;
    int size = 1;

    for (int i = size; i <= size + 20; i++){
        drawSquare(&buffer, i, xOrigin, yOrigin);
        draw(&buffer);
        usleep(10000);
    }

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}