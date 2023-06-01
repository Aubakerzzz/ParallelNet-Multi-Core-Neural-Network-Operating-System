/*

Operating Systems Project

Name: Abubakar
Roll Number: 21I-1379
Section: F/H

*/




#include <iostream>
#include <string>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <mutex>
#include <semaphore.h>
#include <vector>
#include<fstream>

using namespace std;

vector<vector<vector<float>>> hiddenLayer;	//outer most vector layer
vector<vector<float>> midVector; 	//holds another vector of weights
vector<vector<float>> inputLayer; 
vector<float> outputLayer;
vector<float> input;

int layers, neurons;
const int iterations = 2;
int totaliter = 0;
int *fd;
int *globalPipe;
pthread_mutex_t mute;

void FileReading()
{
	string line;
	string num="";
	float number;
	bool inputWeights=false;
	bool hidden=false;
	bool output=false;
	bool inputdataaa=false;
	
	ifstream file("Configuration.txt");
	
	while(getline(file, line))
	{
		vector<float> weights;		
		
   if(line[0]=='I'&&line[1]=='n'&&line[2]=='p'&&line[3]=='u'&&line[4]=='t'&& line[5]==' ' && line[6]=='l' && line[7]=='a' && line[8]=='y' &&line[9]=='e' && line[10]=='r')
		{
			inputWeights=true;
			continue;
		}
		 if(line[0]=='H'&&line[1]=='i'&&line[2]=='d'&&line[3]=='d'&&line[4]=='e'&& line[5]=='n')
		 {
		 	hidden=true;
		 	continue;
		 }
		 
		 if(line[0]=='O'&&line[1]=='u'&&line[2]=='t'&&line[3]=='p'&&line[4]=='u'&& line[5]=='t')
		{
			hidden=false;
			output=true;
			continue;
		}
		
		if(line[0]=='I'&&line[1]=='n'&&line[2]=='p'&&line[3]=='u'&&line[4]=='t'&& line[5]==' ' && line[6]=='d' && line[7]=='a' && line[8]=='t' &&line[9]=='a')
		{
			output=false;
			inputdataaa=true;
			continue;
		}
		
		if(hidden==true && line.length()==1)
		{
			hiddenLayer.push_back(midVector);	
			while(!midVector.empty())	
			{
				midVector.pop_back();
			}
			continue;
		}
		
		if(inputWeights==true && line.length()==1)
		{
			inputWeights=false;
			continue;
		}
		
		if(line[0]!='-')		//condition to skip headings
		{	
			if( line[0]!='1'&& line[0]!='2'&& line[0]!='3'&& line[0]!='4'&& line[0]!='5' && line[0]!='6'&& line[0]!='7'&& line[0]!='8' && line[0]!='9' && line[0]!='.'&& line[0]!='0'&& line[0]!=' ')
			{	
				continue;
			}
		}
			
		for(int i=0; i<=line.length(); i++)	//loop to identify floats and avoid commas,spaces etc
		{
			if(line[i]=='0'|| line[i]=='1'|| line[i]=='2'|| line[i]=='3'|| line[i]=='4'|| line[i]=='5' || line[i]=='6'|| line[i]=='7'|| line[i]=='8' || line[i]=='9' || line[i]=='.' || line[i]=='-'||line[i]==','||line[i]==' ')
			{
				if(line[i]!=','&& line[i]!=' ')	//consider all characters except space and comma
				{
					num=num+line[i];
				}
				
				if(line[i]==',')	//if we get a comma a num has been completed
				{	if(inputdataaa==true)
					{
					number = std::stof(num);
					input.push_back(number);
					num="";
					}
					else if(output==true)
					{
					number = std::stof(num);
					outputLayer.push_back(number);
					num="";
					}
					else
					{
					number = std::stof(num);
					weights.push_back(number);
					num="";
					}
					if(line[i+1]==' ')
						i++; //skip space after comma
				}	
				
				if((line[i]==' '&& line[i+1]==' ')||(line[i]==' ' && line[i+1]=='/')||(line[i]=='/'))	//avoiding comments
				{
					if(inputdataaa==true)
					{
					number = std::stof(num);
					input.push_back(number);
					num="";
					break;
					}
					else if(output==true)
					{
					number = std::stof(num);
					outputLayer.push_back(number);
					num="";
					break;
					}
					else
					{
					number = std::stof(num);
					weights.push_back(number);
					num="";
					break;
					}
				}
				
				if(i==line.length()-2)		//detect end of line
				{	
					if(inputdataaa==true)
					{
					number = std::stof(num);
					input.push_back(number);
					num="";
					}
					else if(output==true)
					{
					number = std::stof(num);
					outputLayer.push_back(number);
					num="";
					
					}
					else
					{
					number = std::stof(num);
					weights.push_back(number);
					num="";
					}
				}	
			}
		}
		if(hidden==true)
		{
			midVector.push_back(weights);
		}
		else if(inputWeights==true)
		{
			inputLayer.push_back(weights);
		}
	}
	file.close();
}

struct inputs
{
    float *inps;
    float *result;
    bool *calculated;
    float *out;
};

struct dataaa
{
    int layer;
    int pos;
    inputs *in;
};

void *neuron(void *nargs)
{
    dataaa *d = (dataaa *)nargs;

    pthread_mutex_lock(&mute);
    float val = 0;

    if (d->layer == 1)
    { // For starting layer
        for (int i = 0; i < input.size(); i++)
        {
            // cout << input[i] << " * " << inputLayer[i][d->pos] << " = " << input[i] * inputLayer[i][d->pos] << endl;
            val += input[i] * inputLayer[i][d->pos];
        }

        d->in->inps[d->pos] = val;
        d->in->calculated[d->pos] = true;
        // cout << d->in->inps[d->pos] << "\t" << d->in->calculated[d->pos] << "\t" << val << "\t" << d->pos << endl;
    }

    else if (d->layer == layers)
    { // For ending layer // WOrk remining
        for (int i = 0; i < neurons; i++)
        {
            // cout << d->in->inps[i] << " * " << outputLayer[i] << " = " << d->in->inps[i] * outputLayer[i] << endl;
            val += d->in->inps[i] * outputLayer[i];
        }

        d->in->out[d->pos] = val;
        d->in->calculated[d->pos] = true;
    }

    else
    { // For hidden layers
        for (int i = 0; i < neurons; i++)
        {
            // cout << d->in->inps[i] << " * " << hiddenLayer[d->layer - 2][d->pos][i] << " = " << d->in->inps[i] * hiddenLayer[d->layer - 2][d->pos][i] << endl;
            val += d->in->inps[i] * hiddenLayer[d->layer - 2][d->pos][i];
        }

        d->in->result[d->pos] = val;
        d->in->calculated[d->pos] = true;
        // cout << d->in->result[d->pos] << "\t" << d->pos << endl;
    }

    pthread_mutex_unlock(&mute);

    pthread_exit(NULL);
}

void layering(int layer)
{
    pthread_attr_t neurons_attr;
    pthread_t *neurons_pid = new pthread_t[neurons];

    pthread_attr_init(&neurons_attr);
    pthread_attr_setdetachstate(&neurons_attr, PTHREAD_CREATE_DETACHED);

    inputs *in = new inputs;
    in->inps = new float[neurons];
    in->result = new float[neurons];
    in->calculated = new bool[neurons];

    if (layer != 1)
    {
        int temp[2];
        pipe(temp);

        if (fork() == 0)
        {
            close(fd[1]);
            uintptr_t ptr_value;
            read(fd[0], &ptr_value, sizeof(ptr_value));

            close(temp[0]);
            write(temp[1], &ptr_value, sizeof(ptr_value));
            exit(0);
        }

        else
        {
            uintptr_t ptr_value;
            close(temp[1]);
            read(temp[0], &ptr_value, sizeof(ptr_value));
            in = reinterpret_cast<inputs *>(ptr_value);
        }
    }

    if (layer == layers)
    {
        in->out = new float[input.size()];

        for (int i = 0; i < input.size(); i++)
        {
            dataaa *d = new dataaa;
            d->in = in;
            d->layer = layer;
            d->pos = i;

            in->calculated[i] = false;
            pthread_create(&neurons_pid[i], &neurons_attr, neuron, d);
        }

        for (int i = 0; i < neurons; i++)
        {
            if (in->calculated[i] == false)
            {
                i = 0;
            }
        }

        float x1 = in->out[0];
        x1 = ((x1 * x1) + x1 + 1) / 2.0;
        in->out[0] = x1;

        float x2 = in->out[1];
        x2 = ((x2 * x2) - x2) / 2.0;
        in->out[1] = x2;

        cout << "Layer Number " << layer << endl
             << endl;
        for (int i = 0; i < input.size(); i++)
        {
            cout << "Neuron " << i + 1 << ": " << in->out[i] << endl;
        }

        float ret[2] = {x1, x2};
        int lp[2];
        pipe(lp);
        bool cp = 1;

        if (fork() == 0)
        {
            close(globalPipe[0]);
            write(globalPipe[1], ret, sizeof(ret));

            close(lp[0]);
            write(lp[1], &cp, sizeof(cp));
            exit(0);
        }

        else
        {
            close(lp[1]);
            read(lp[0], &cp, sizeof(cp));
        }
    }

    else
    {
        for (int i = 0; i < neurons; i++)
        {
            dataaa *d = new dataaa;
            d->in = in;
            d->layer = layer;
            d->pos = i;

            in->calculated[i] = false;
            pthread_create(&neurons_pid[i], &neurons_attr, neuron, d);
        }

        for (int i = 0; i < neurons; i++)
        {
            if (in->calculated[i] == false)
            {
                i = 0;
            }
        }

        if (layer != 1)
        {
            for (int i = 0; i < neurons; i++)
                in->inps[i] = in->result[i];
        }

        cout << "Layer Number " << layer << endl
             << endl;
        for (int i = 0; i < neurons; i++)
        {
            cout << "Neuron " << i + 1 << ": " << in->inps[i] << endl;
        }
    }

    // getchar();
    int temp2[2];
    pipe(temp2);

    if (fork() == 0)
    {
        close(fd[0]);
        uintptr_t ptr_value = reinterpret_cast<uintptr_t>(in);
        write(fd[1], &ptr_value, sizeof(ptr_value));

        close(temp2[0]);
        bool done1 = true;
        write(temp2[1], &done1, sizeof(done1));

        exit(0);
    }

    else
    {
        close(temp2[1]);
        bool done1;
        read(temp2[0], &done1, sizeof(done1));

        if (layer + 1 <= layers) layering(layer + 1);
        else totaliter++;
        if (totaliter == 3) exit(0);

        float ret[2];
        int pls[2], go[2];
        pipe(pls);
        pipe(go);

        // 1 process

        if (fork() == 0) {
            // 2 process
            close(globalPipe[1]);
            read(globalPipe[0], ret, sizeof(ret));

            cout << "Backward Propogation at layer " << layer << endl;
            for (int i = 0; i < 2; i++)
            {
                cout << ret[i] << endl;
            }
            cout << endl;

            close(pls[0]);
            write(pls[1], ret, sizeof(ret));
            exit(0); // 1 process
        }

        else {
            close(pls[1]);
            read(pls[0], ret, sizeof(ret));

            if (fork() == 0) {
                close(globalPipe[0]);
                write(globalPipe[1], ret, sizeof(ret));

                bool written = false;
                close(go[0]);
                write(go[1], &written, sizeof(written));
                exit(0);
            }

            else {
                bool written;
                close(go[1]);
                read(go[0], &written, sizeof(written));
            }
        }

        if (layer == 1) {
            input[0] = ret[0];
            input[1] = ret[1];
            totaliter++;
        }
    }
}

int main()
{
    FileReading();
    
    /* //code to display vectors ki values
     cout<<"hidden layers:"<<endl;
	for(int i=0; i<hiddenLayer.size(); i++)
	{
		for(int j=0; j<hiddenLayer[i].size(); j++)
		{
			for(int k=0; k<hiddenLayer[i][j].size(); k++)
			{
				cout<<hiddenLayer[i][j][k]<<" ";
			}
		cout<<endl;
		}
	cout<<endl;
	}
	
	cout<<"input layer:"<<endl;
	for(int i=0 ; i<inputLayer.size(); i++)
	{
		for(int j=0; j<inputLayer[i].size(); j++)
		{
			cout<<inputLayer[i][j]<<" ";
		}
		cout<<endl;
	}
	
	cout<<"output layer:"<<endl;
	for(int i=0; i<outputLayer.size(); i++)
	{
		cout<<outputLayer[i]<<" ";
	
	}
	
	cout<<endl;
	cout<<"input dataaa layer:"<<endl;
	for(int i=0 ; i<input.size(); i++)
	{
		cout<<input[i]<<" ";
	
	}*/

    do { 
        cout << "Enter Number of Layers between (2 - " << hiddenLayer.size() + 2  << "): ";
        cin >> layers;
    } while (layers < 2 || layers > hiddenLayer.size() + 2);

    do {
        cout << "Enter Number of Neurons between (2 - " <<  outputLayer.size() << "): ";
        cin >> neurons;
    } while (neurons < 2 || neurons > outputLayer.size());

    int iter = 0, num = 1;
    
    getchar();
    while (iter < iterations)
    {
        fd = new int[2];
        globalPipe = new int[2];
        pipe(fd);
        pipe(globalPipe);
        num = 1;
        layering(num);
        iter++;
    }
}

 
    
