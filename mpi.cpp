#include "asg2.h"
#include <stdio.h>
#include <mpi.h>


int rank;
int world_size;
int size_local; // nof point for evert thread = total size / nof pro

Point* data_local;

int main(int argc, char *argv[]) {
	if ( argc == 4 ) {
		X_RESN = atoi(argv[1]);
		Y_RESN = atoi(argv[2]);
		max_iteration = atoi(argv[3]);
	} else {
		X_RESN = 1000;
		Y_RESN = 1000;
		max_iteration = 100;
	}

	if (rank == 0) {
		#ifdef GUI
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		glutInitWindowSize(500, 500); 
		glutInitWindowPosition(0, 0);
		glutCreateWindow("MPI");
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glMatrixMode(GL_PROJECTION);
		gluOrtho2D(0, X_RESN, 0, Y_RESN);
		glutDisplayFunc(plot);
		#endif
	}

	/* computation part begin */
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	total_size = X_RESN * Y_RESN;
	size_local = total_size/world_size;
	// data_local = new Point[size_local]; // must exact division

	//float color_local[size_local];
	float color_total[total_size];
	
	if (rank == 0){
		initData();
		//data_local = new Point[size_local]; // must exact division
		printf("\n======= MPI START ======\n\n");
		t1 = std::chrono::high_resolution_clock::now();
	}
	if(rank == 0){
        printf("Total = %d, Size local = %d\n\n", total_size,size_local);
    }else{
        printf("Thread %d report: Total = %d, Size local = %d\n\n",rank, total_size,size_local);
    }
	MPI_Barrier(MPI_COMM_WORLD);

	// printf("\nHere 1 !!!!\n\n");

	data_local = new Point[size_local];
	//Point data_local[size_local]; // this is not enough when size > 1000^2
	// printf("\nHere 2 !!!!\n\n");
	
	// ==== TO DO ==== //
	// SHould change to new, not directly create arry
	float colorResult_local[size_local]; //This is not enough when size > 1100 
	// printf("\nHere 3 !!!!\n\n");
	int nofcl = 0;
	Point* p_local = data_local;
	// printf("\nHere 4 !!!!\n\n");
	//printf("data local 1 = %d from p%d\n", p_local->x, rank);

	// No scatter, each thread chose its rows by rank one by one 
	// solely store color information to a Float Array to simplify MPI_Gather
	for (int i = rank; i < X_RESN; i=i+world_size){
		for(int j=0; j<Y_RESN; j++){
			p_local->x = i;
			p_local->y = j;

			compute(p_local);
			if(i==1 && j==0){p_local->color = 8.88;}
			colorResult_local[nofcl] = p_local->color;
			
			//printf("X=%d Y=%d, p_loco color = %lf\n",i,j, p_local->color);
			//printf("No.%d color local = %lf\n", nofcl, colorResult_local[nofcl]);

			if(nofcl == size_local-1){
				printf("No.%d color local = %lf\n", nofcl, colorResult_local[nofcl]);
			}

			p_local++;
			nofcl++;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);

	//send & recv way
	// float dataRecvfrom1[size_local];
	// if(rank==0){
	// 	MPI_Recv(dataRecvfrom1, size_local, MPI_FLOAT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// 	for(int i=0; i<size_local;i++){
	// 		printf("\ndata tecv from P1 No.%d = %lf\n", i,dataRecvfrom1[i]);
	// 	}
	// 	printf("\ndata tecv from P1 No.0 = %lf\n\n", dataRecvfrom1[0]);
	// }else{
	// 	//printf("\ndata send to P0 No.0 = %lf\n\n", colorResult_local[0]);
	// 	MPI_Send(colorResult_local, size_local, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
	// }
	// printf("send & recv sucess of p%d\n", rank);

	//Mater thread gather color by Float Array
	MPI_Gather(colorResult_local, size_local, MPI_FLOAT, color_total, size_local, MPI_FLOAT, 0, MPI_COMM_WORLD);

	//print gather result success
	if(rank == 0) {
		printf("\nStart print gather data!!\n");
		for(int i=0; i<total_size; i++){
			if(i == total_size -1){
				printf("i = %d, total size = %d\n", i, total_size);
				printf("Gather data%d = %lf\n", i, color_total[i]);
			}
		}
	}else{
		printf("slave thread %d wait master\n", rank);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	// store color back to "data" to correctly show color in GUI
	if (rank == 0){
		//printf("final process of data load!!\n\n");
		int n, rankP, nofGroup;
		Point* p_total = data;
		for(int i=0; i<X_RESN; i++){
			rankP  = i%world_size; //this row from which rank of processer
			nofGroup = i/world_size+1; //which group of y tern of above p
			n = total_size/world_size*rankP + (nofGroup-1)*Y_RESN;
			//locate the pos of data(2d) in color array(one dimention)
			
			for(int j=0; j<Y_RESN; j++){

				p_total->color = color_total[n+j]; //each data point locate in color array
				//printf("X=%d, Y=%d, color=%lf\n",p_total->x, p_total->y, p_total->color);
				p_total++;
			}
		}

		t2 = std::chrono::high_resolution_clock::now();  
		time_span = t2 - t1;
	}

	if (rank == 0){
		printf("\nStudent ID: 117010332\n"); // replace it with your student id
		printf("Name: XU Jiale\n"); // replace it with your name
		printf("Assignment 2 MPI\n");
		printf("Run Time: %f seconds\n", time_span.count());
		printf("Problem Size: %d * %d, %d\n", X_RESN, Y_RESN, max_iteration);
		printf("Process Number: %d\n\n", world_size);
	}

	MPI_Finalize();
	/* computation part end */

	if (rank == 0){
		#ifdef GUI
		glutMainLoop();
		#endif
	}

	return 0;
}
