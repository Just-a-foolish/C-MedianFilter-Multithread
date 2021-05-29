#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

/*---------------------------------------------------------------------*/
#pragma pack(1)

/*---------------------------------------------------------------------*/
struct cabecalho {
	unsigned short tipo;
	unsigned int tamanho_arquivo;
	unsigned short reservado1;
	unsigned short reservado2;
	unsigned int offset;
	unsigned int tamanho_image_header;
	int largura;
	int altura;
	unsigned short planos;
	unsigned short bits_por_pixel;
	unsigned int compressao;
	unsigned int tamanho_imagem;
	int largura_resolucao;
	int altura_resolucao;
	unsigned int numero_cores;
	unsigned int cores_importantes;
}; 
typedef struct cabecalho CABECALHO;

struct rgb{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
};
typedef struct rgb RGB;

struct argumentos{
	int largura;
	int inicio;
	int final;
	RGB *vetor;
	int filtro;
};
typedef struct argumentos ARGUMENTOS;

/*---------------------------------------------------------------------*/

int compare_function(const void *a, const void *b){
	int *x = (int *) a;
	int *y = (int *) b;
	return *x - *y;
}

void * filtroMediana(void * args){ 
	ARGUMENTOS *p = (ARGUMENTOS *) args;
	
        int largura = p->largura; 
        int inicio = p->inicio; 
        int final = p->final;
        RGB *vetor = p->vetor; 
        int filtro = p->filtro;
	
	int matrizAuxBlue[500];
	int matrizAuxGreen[500];
	int matrizAuxRed[500];
	int i, j, k, l;
	int a = 0;
	int indice = filtro/2;
	int meio = ((filtro*filtro)/2);
	int tamanhoSort = (filtro*filtro)*4;
		
	for(i=inicio+4; i<final-filtro; i++){
	  for(j=filtro-2; j<largura-filtro; j++){
	    for(k=indice*(-1); k<=indice; k++){
	      for(l=indice*(-1); l<=indice; l++){
	     	matrizAuxRed[a]  =vetor[(k*largura) + (i*largura+j) + l].red;
	     	matrizAuxGreen[a]=vetor[(k*largura) + (i*largura+j) + l].green;
	     	matrizAuxBlue[a] =vetor[(k*largura) + (i*largura+j) + l].blue;
	     	a++; 
	     }
	  }
	  a=0;
	  qsort(matrizAuxRed, tamanhoSort, sizeof(*matrizAuxRed), compare_function);
	  qsort(matrizAuxBlue, tamanhoSort, sizeof(*matrizAuxBlue), compare_function);
	  qsort(matrizAuxGreen, tamanhoSort, sizeof(*matrizAuxGreen), compare_function);
	  vetor[i * largura + j].red = matrizAuxRed[meio];
	  vetor[i * largura + j].blue = matrizAuxBlue[meio];
	  vetor[i * largura + j].green = matrizAuxGreen[meio];	  
	 }
	  vetor++;    	
	}
}


int main(int argc, char **argv ){	
		
	// DADOS DE INICIALIZAÇÃO
	char entrada[] = "borboleta.bmp";
	char saida[] = "teste";
	int quantThreads = 4;
	int tamanhoMascara = 7;
	
/*---------------------------------------------------------------------*/
	CABECALHO cabecalho;
	RGB pixel;
	int i, j;
	pthread_t tid[quantThreads];
	ARGUMENTOS *args = NULL;
	int alturaIndividual, finalIndividual, k;
	args = (ARGUMENTOS *)malloc(quantThreads * sizeof(ARGUMENTOS));
	
		
	FILE *fin = fopen(entrada, "rb");

	if ( fin == NULL ){
		printf("Erro ao abrir o arquivo %s\n", entrada);
		exit(0);
	}  

	FILE *fout = fopen(saida, "wb");

	if ( fout == NULL ){
		printf("Erro ao abrir o arquivo %s\n", saida);
		exit(0);
	}  

	// lê e escreve o cabeçalho no novo arquivo
	fread(&cabecalho, sizeof(CABECALHO), 1, fin);
	fwrite(&cabecalho, sizeof(CABECALHO), 1, fout);
	
	// aloca a matriz da imagem em um vetor
	RGB *vetor = (RGB *) malloc(cabecalho.altura * cabecalho.largura * sizeof(RGB));
	
	for(i=0; i<cabecalho.altura; i++){
		int ali = (cabecalho.largura * 3) % 4;
		
		if (ali != 0){
			ali = 4 - ali;
		}

		for(j=0; j<cabecalho.largura; j++){
			fread(&vetor[i * cabecalho.largura + j], sizeof(RGB), 1, fin);
		}
		
		unsigned char aux;
		for(j=0; j<ali; j++){
			fwrite(&aux, sizeof(unsigned char), 1, fout);
		}
	}
	
	alturaIndividual = cabecalho.altura/quantThreads;
	finalIndividual = cabecalho.altura/quantThreads;
	for(i = 0; i<quantThreads; i++){
		args[i].largura = cabecalho.largura;
		args[i].inicio = alturaIndividual*i;
		args[i].final = finalIndividual*(i+1);
		args[i].vetor = vetor;
		args[i].filtro = tamanhoMascara;
	}
			
	for(i=0; i<quantThreads; i++){
		pthread_create(&tid[i], NULL, filtroMediana, (void *)&args[i]);
	}
		
	for(i=0; i<quantThreads; i++){
		pthread_join(tid[i], NULL);
	} 	 	
			
	// escreve o vetor com o filtro aplicado no arquivo de saída
	for(i=0; i<cabecalho.altura; i++){
		int ali = (cabecalho.largura * 3) % 4;		
		if (ali != 0){
			ali = 4 - ali;
		}

		for(j=0; j<cabecalho.largura; j++){
			fwrite(&vetor[i * cabecalho.largura + j], sizeof(RGB), 1, fout);
		}
		
		unsigned char aux;
		for(j=0; j<ali; j++){
			fwrite(&aux, sizeof(unsigned char), 1, fout);
		}
	}	

	free(vetor);
	fclose(fin);
	fclose(fout);
	
		
}
/*---------------------------------------------------------------------*/


