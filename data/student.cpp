#include<iostream>
using namespace std;

string arr1[20], arr2[20], arr3[20], arr4[20], arr5[20], arr6[20], arr7[20];
int total=0;
void enter()
{
    int ch=0;
    cout<<"How many students do you want to enroll?"<<endl;
    cin>>ch;
    if(total==0)
    {
        total = ch+total;
        for(int i=0;i<ch;i++)
        {
            cout<<"\nEnter the Data of the student"<<i+1<<endl<<endl;
            cout<<"Enter name: ";
            cin>>arr1[i];
             cout<<"Enter Roll no: ";
            cin>>arr2[i];
             cout<<"Enter Course: ";
            cin>>arr3[i];
             cout<<"Enter Class: ";
            cin>>arr4[i];
             cout<<"Enter Section: ";
            cin>>arr5[i]; 
            cout<<"Enter Contact: ";
            cin>>arr6[i];
             cout<<"Enter Address: ";
            cin>>arr7[i];

            
        }
    }
    else{
        for(int i=total;i<ch+total;i++)
        {
         cout<<"\nEnter the Data of the student"<<i+1<<endl<<endl;
            cout<<"Enter name: ";
            cin>>arr1[i];
             cout<<"Enter Roll no: ";
            cin>>arr2[i];
             cout<<"Enter Course: ";
            cin>>arr3[i];
             cout<<"Enter Class: ";
            cin>>arr4[i];
             cout<<"Enter Section: ";
            cin>>arr5[i]; 
            cout<<"Enter Contact: ";
            cin>>arr6[i];
             cout<<"Enter Address: ";
            cin>>arr7[i];   
        }
        total=ch+total;
    }
}

void show()
{
    if(total==0)
    {
        cout<<"No data is entered"<<endl;
    }
    else{
        for(int i;i<total;i++)
        {
            cout<<"\nData of Student"<<i+1<<endl<<endl;
            cout<<"Name"<<arr1[i]<<endl;
            cout<<"Roll no"<<arr2[i]<<endl;
            cout<<"course"<<arr3[i]<<endl;
            cout<<"Class"<<arr4[i]<<endl;
            cout<<"Section"<<arr5[i]<<endl;
            cout<<"Contact"<<arr6[i]<<endl;
            cout<<"Address"<<arr7[i]<<endl;

        }
    }
}

void search()
{
    if(total==0)
    {
        cout<<"No data is entered"<<endl;
    }
    else{
        string rollno;
        cout<<"Enter the roll no of student"<<endl;
        cin>>rollno;
         for(int i=0;i<total;i++)
         {
            if(rollno==arr2[i])
            {
             cout<<"Name"<<arr1[i]<<endl;
            cout<<"Roll no"<<arr2[i]<<endl;
            cout<<"course"<<arr3[i]<<endl;
            cout<<"Class"<<arr4[i]<<endl;
            cout<<"Section"<<arr5[i]<<endl;
            cout<<"Contact"<<arr6[i]<<endl;
            cout<<"Address"<<arr7[i]<<endl;   
            }
         }
   }
}

void update()
{
    if(total==0)
    {
        cout<<"No data is entered"<<endl;
    }
    else{
        string rollno;
        cout<<"Enter the roll number which you want to update"<<endl;
        cin>>rollno;
        for(int i=0; i<total; i++)
        {
            if(rollno==arr2[i])
            {
            cout<<"\nPrevious data"<<endl<<endl;
            cout<<"Data of Student"<<i+1<<endl;
            cout<<"Name"<<arr1[i]<<endl;
            cout<<"Roll no"<<arr2[i]<<endl;
            cout<<"course"<<arr3[i]<<endl;
            cout<<"Class"<<arr4[i]<<endl;
            cout<<"Section"<<arr5[i]<<endl;
            cout<<"Contact"<<arr6[i]<<endl;
            cout<<"Address"<<arr7[i]<<endl;   
            cout<<"\nEnter new data"<<endl<<endl;
            cout<<"Enter name: ";
            cin>>arr1[i];
             cout<<"Enter Roll no: ";
            cin>>arr2[i];
             cout<<"Enter Course: ";
            cin>>arr3[i];
             cout<<"Enter Class: ";
            cin>>arr4[i];
             cout<<"Enter Section: ";
            cin>>arr5[i]; 
            cout<<"Enter Contact: ";
            cin>>arr6[i];
             cout<<"Enter Address: ";
            cin>>arr7[i];   
            }
        }
    }
}

void deleterecord()
{
    if(total==0)
    {
        cout<<"No data is entered"<<endl;
    }
    else{
        int a;
        cout<<"Press 1 to delete all record"<<endl;
        cout<<"Press 2 to delete specific record"<<endl;
        cin>>a;
        if(a==1)
        {
            total=0;
            cout<<"All record is deleted..!!"<<endl;
        }
        else if(a==2)
        {
            string rollno;
            cout<<"Enter the roll number of student which you wanted to delete"<<endl;
            cin>>rollno;
            for(int i=0; i<total; i++)
            {
                if(rollno==arr2[i])
                {
                    for(int j=i; j<total; j++)
                    {
                        arr1[j]=arr1[j+1];
                        arr2[j]=arr2[j+1];
                        arr3[j]=arr3[j+1];
                        arr4[j]=arr4[j+1];
                        arr5[j]=arr5[j+1];
                        arr6[j]=arr6[j+1];
                        arr7[j]=arr7[j+1];
                    }
                    total--;
                    cout<<"Your required record is deleted"<<endl;
                }
            }
        }
        else{
            cout<<"Invalid input";
        }

    }
}

 main()
 {
    int value;
    while(true)
    {
        cout<<"\nPress 1 to enter data"<<endl;
        cout<<"Press 2 to show data"<<endl;
        cout<<"Press 3 to search data"<<endl;
        cout<<"Press 4 to update data"<<endl;
        cout<<"Press 5 to delete data"<<endl;
        cout<<"Press 6 to exit"<<endl;
        cout<<"Enter your choice: ";
        cin>>value;
        switch (value)
        {
        case 1:
            enter();
            break;
        case 2:
            show();
            break; 

        case 3:
            search();
            break;    
        case 4:
            update();
            break;
        case 5:
            deleterecord();
            break;
        case 6:
            exit(0);
            break;

            default:
            cout<<"Invalid input"<<endl;
            break;
            
        }
    }
 }
