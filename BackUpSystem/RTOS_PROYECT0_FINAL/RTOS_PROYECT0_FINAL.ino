#include <Arduino_FreeRTOS.h>
int t_i;
int t_o;
int n_r;
boolean e_1=0;
boolean e_2=0;


// se definen las dos tareas principales
void Task_e_1( void *pvParameters );
void Taskturbidez_1( void *pvParameters );




void setup() {//se inicializan las comunicaciones seriales para las pruebas de montaje
  

  Serial.begin(9600);
  
  while (!Serial) {
    ;
  }

  // se declaran 2 tareas

  xTaskCreate(
    Taskturbidez_1   //nombre de la tarea1
    ,  "turbidez_1"//
    ,  128  // Stack size
    ,  NULL
    ,  3 // prioridad 3, prioridad alta
    ,  NULL );


  
  xTaskCreate(
    Task_e_1
    ,  "e_1"   // nommre de la tarea 2, de alarmas
    ,  128  // 
    ,  NULL
    ,  2 // prioridad 2, intermedia
    ,  NULL );



  // 
}

void loop()
{


  

     
  
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/



void Taskturbidez_1(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  const int led=13;
  const int nivel=6;
  int valor=0;
 const int ev_1=10;
 
 
pinMode(ev_1,OUTPUT);
pinMode(led,OUTPUT);
pinMode(nivel,INPUT);


  for (;;)
  {
   
   valor=digitalRead(nivel);
      printf(valor);
   if(valor==0){//si la turbidez entrada es mayor al 40% _______A0
   
   
      
      e_1=1;//apague electrovalvula principal
      e_2=1;//encender electrovalvula de llenado
       digitalWrite(ev_1, HIGH);  // statements
   

   
   
          

    }  

    if(valor==1){
        e_1=0;//apague electrovalvula principal
       digitalWrite(ev_1, LOW);  // statements
      }

  

     if((e_1==1)){    //alarma_1 hay turbidez entrada y está lleno el tanque
          digitalWrite(led, HIGH);   // 
          vTaskDelay( 250 / portTICK_PERIOD_MS ); // blink de alarma
          digitalWrite(led, LOW);    // 
          vTaskDelay( 250 / portTICK_PERIOD_MS ); // 
         }

         else{
           digitalWrite(led, LOW);  
          }
     
      }
     }






void Task_e_1(void *pvParameters)  // This is a task.
{
  (void) pvParameters;




  const int ev_1=10;
 // pinMode(ev_1,OUTPUT);

  for (;;) //loop
  {


 if (e_1==0){
  digitalWrite(ev_1, LOW);  // statements
  }

   if (e_1==1){
  digitalWrite(ev_1, HIGH);  // statements
  }
     

     
    }
   }


 
 
    
 
