// Create other necessary functions here
__global__ void gpuMultiplication(int N, int *matA, int *matB, int *output)
{
	int out = 0LL;
	int row = threadIdx.y + blockIdx.y * blockDim.y;
	int col = threadIdx.x + blockIdx.x * blockDim.x;
	if( (row<<1LL) + 2 <= N and (col<<1LL) + 2 <= N){
		for (int k = 0;k < N; k++){
			out += matA[(row << 1LL) * N + k] * matB[k*N + (col << 1LL)];
			out += matA[(row << 1LL) * N + k] * matB[k*N + (col << 1LL) + 1];
			out += matA[((row << 1LL) + 1) * N + k] * matB[k*N + (col << 1LL)];
			out += matA[((row << 1LL) + 1) * N + k] * matB[k*N + (col << 1LL) + 1];
		}
		output[1LL * row * (N>>1LL) + col] = out;
	}
} 

// Fill in this function
void gpuThread(int N, int *matA, int *matB, int *output)
{
	
    	cudaDeviceSynchronize();
	int *output_gpu = new int[(N>>1LL)*(N>>1LL)];
	for(int i=0;i<(N>>1LL)*(N>>1LL);i++)
	    output_gpu[i]=0;

	chrono::time_point<chrono::system_clock> start_gpu, end_gpu;
	start_gpu = chrono::system_clock::now();

	int BLOCK_DIM = 1<<1LL;
//	int threadsx = BLOCK_DIM * BLOCK_DIM;
//	if((N>>1LL)*(N>>1LL) < threadsx){
//	       BLOCK_DIM = 1<<2;
//	}	       
	int threadsx = BLOCK_DIM * BLOCK_DIM;
	int sizeOutput = 1LL * (N>>1LL) * (N>>1LL);
	int GRID_DIM = 1LL * (sizeOutput/threadsx);
	//int GRID_DIMx = 1LL * GRID_DIM;
	dim3 blocksPerGrid(sqrt(GRID_DIM),sqrt(GRID_DIM), 1);
	dim3 threadsPerBlock(BLOCK_DIM, BLOCK_DIM, 1);

	int *d_MatA, *d_MatB, *d_output;
	cudaMalloc(&d_MatA,1LL *  N * N * sizeof(int));
	cudaMalloc(&d_MatB, 1LL * N * N * sizeof(int));
	cudaMalloc(&d_output, 1LL * (N>>1LL) * (N>>1LL) * sizeof(int));

	cudaMemcpy(d_MatA, matA,1LL *  N * N * sizeof(int) , cudaMemcpyHostToDevice);
	cudaMemcpy(d_MatB, matB, 1LL * N * N * sizeof(int) , cudaMemcpyHostToDevice);
	cudaMemcpy(d_output, output ,1LL * (N>>1LL) * (N>>1LL) * sizeof(int) ,cudaMemcpyHostToDevice);

	cudaDeviceSynchronize();
	gpuMultiplication<<<blocksPerGrid, threadsPerBlock>>>(N ,d_MatA, d_MatB, d_output);
	cudaDeviceSynchronize();

	cudaMemcpy(output, d_output,1LL * (N>>1LL) * (N>>1LL) * sizeof(int) ,cudaMemcpyDeviceToHost);

	cudaFree(d_MatA);
	cudaFree(d_MatB);
	cudaFree(d_output);

	end_gpu = chrono::system_clock::now();
	chrono::duration<double> elapsed_seconds_gpu = end_gpu - start_gpu;
	cout<<"Elapsed Time for GPU in milliseconds :: "<<(elapsed_seconds_gpu.count())*1000<< endl;

	cudaDeviceSynchronize();
	
}
