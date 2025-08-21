#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "seamcarving.h"


#define min(a, b) ((a) < (b) ? (a) : (b))


double dy(struct rgb_img *im, int y, int x){
    if (y==0){
        int R = get_pixel(im,im->height-1,x,0)-get_pixel(im,y+1,x,0);
        int G = get_pixel(im,im->height-1,x,1)-get_pixel(im,y+1,x,1);
        int B = get_pixel(im,im->height-1,x,2)-get_pixel(im,y+1,x,2);
        return R*R+G*G+B*B;
    } else if (y==im->height-1){
        int R = -get_pixel(im,y-1,x,0)+get_pixel(im,0,x,0);
        int G = -get_pixel(im,y-1,x,1)+get_pixel(im,0,x,1);
        int B = -get_pixel(im,y-1,x,2)+get_pixel(im,0,x,2);
        return R*R+G*G+B*B;
    } else {
        int R = get_pixel(im,y+1,x,0)-get_pixel(im,y-1,x,0);
        int G = get_pixel(im,y+1,x,1)-get_pixel(im,y-1,x,1);
        int B = get_pixel(im,y+1,x,2)-get_pixel(im,y-1,x,2);
        return R*R+G*G+B*B;
    }
}

double dx(struct rgb_img *im, int y, int x){
    if (x==0){
        int R = get_pixel(im,y,im->width-1,0)-get_pixel(im,y,x+1,0);
        int G = get_pixel(im,y,im->width-1,1)-get_pixel(im,y,x+1,1);
        int B = get_pixel(im,y,im->width-1,2)-get_pixel(im,y,x+1,2);
        return R*R+G*G+B*B;
    } else if (x==im->width-1){
        int R = -get_pixel(im,y,x-1,0)+get_pixel(im,y,0,0);
        int G = -get_pixel(im,y,x-1,1)+get_pixel(im,y,0,1);
        int B = -get_pixel(im,y,x-1,2)+get_pixel(im,y,0,2);
        return R*R+G*G+B*B;
    } else {
        int R = get_pixel(im,y,x+1,0)-get_pixel(im,y,x-1,0);
        int G = get_pixel(im,y,x+1,1)-get_pixel(im,y,x-1,1);
        int B = get_pixel(im,y,x+1,2)-get_pixel(im,y,x-1,2);
        return R*R+G*G+B*B;
    }
}
void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    create_img(grad, im->height, im->width);
    for (int y=0; y<im->height; y++){
        for (int x=0; x<im->width; x++){
            double deltay = dy(im,y,x);
            double deltax = dx(im,y,x);
            double energy = sqrt(deltax+deltay);
            double newE = energy/10;
            double realE = (uint8_t)(newE); 
            set_pixel(*grad, y, x, realE, realE, realE);
        }
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    *best_arr = (double *)malloc(sizeof(double) * grad->width * grad->height);
    for (int i =0; i<grad->width; i++){
        (*best_arr)[i] = get_pixel(grad,0,i,0);
    }
    for (int y=1; y<grad->height; y++){
        for (int x=0; x<grad->width; x++){
            if (x==0){
                (*best_arr)[y*grad->width+x] = get_pixel(grad,y,x,0) + min((*best_arr)[(y-1)*grad->width+x], (*best_arr)[(y-1)*grad->width+x+1]);
            } else if (x==grad->width-1){
                (*best_arr)[y*grad->width+x] = get_pixel(grad,y,x,0) + min((*best_arr)[(y-1)*grad->width+x], (*best_arr)[(y-1)*grad->width+x-1]);
            } else {
                (*best_arr)[y*grad->width+x] = get_pixel(grad,y,x,0) + min(min((*best_arr)[(y-1)*grad->width+x], (*best_arr)[(y-1)*grad->width+x+1]), (*best_arr)[(y-1)*grad->width+x-1]);
            }
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    *path = (int*)malloc(height*sizeof(int));
    int min_col = 0;
    double min_val = best[(height - 1) * width + 0];
    for (int j = 1; j < width; j++) {
        double val = best[(height - 1) * width + j];
        if (val < min_val) {
            min_val = val;
            min_col = j;
        }
    }
    (*path)[height - 1] = min_col;
    for (int i = height - 2; i >= 0; i--) {
        int prev = (*path)[i + 1];
        int best_col = prev;
        double best_cost = best[i * width + prev];
        if (prev - 1 >= 0) {
            double left_cost = best[i * width + (prev - 1)];
            if (left_cost < best_cost) {
                best_cost = left_cost;
                best_col = prev - 1;
            }
        }
        if (prev + 1 < width) {
            double right_cost = best[i * width + (prev + 1)];
            if (right_cost < best_cost) {
                best_cost = right_cost;
                best_col = prev + 1;
            }
        }
        (*path)[i] = best_col;
    }
    

}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path)
{
    create_img(dest, src->height, src->width - 1);
    for (int i = 0; i < src->height; i++) {
        int seam_col = path[i]; 
        for (int j = 0; j < seam_col; j++) {
            uint8_t r = get_pixel(src, i, j, 0);
            uint8_t g = get_pixel(src, i, j, 1);
            uint8_t b = get_pixel(src, i, j, 2);
            set_pixel(*dest, i, j, r, g, b);
        }
        for (int j = seam_col + 1; j < src->width; j++) {
            uint8_t r = get_pixel(src, i, j, 0);
            uint8_t g = get_pixel(src, i, j, 1);
            uint8_t b = get_pixel(src, i, j, 2);
            set_pixel(*dest, i, j - 1, r, g, b);
        }
    }
}
/**int main(void){
    struct rgb_img *im = NULL;
    struct rgb_img *grad = NULL;
    double *best_arr = NULL;
    read_in_img(&im,"6x5.bin");
    printf("height: %zu, width: %zu\n", im->height, im->width);
    for (int i=0; i<im->height; i++){
        for (int j=0; j<im->width; j++){
            printf("%d ", get_pixel(im,i,j,0));
        }
        printf("\n");
    }
    calc_energy(im, &grad);
    printf("height: %zu, width: %zu\n", grad->height, grad->width);
    for (int i=0; i<grad->height; i++){
        for (int j=0; j<grad->width; j++){
            printf("%f ", get_pixel(grad,i,j,0));
        }
        printf("\n");
    }
    dynamic_seam(grad, &best_arr);
    for (int i=0; i<im->height; i++){
        for (int j=0; j<im->width; j++){
            printf("%f ", best_arr[i*im->width+j]);
        }
        printf("\n");
    }
    int *path = NULL;
    recover_path(best_arr, im->height, im->width, &path);
    for (int i=0; i<im->height; i++){
        printf("%d ", path[i]);
    }
    return 0;
}**/