#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

// La función que define un scheduler está compuesta por los siguientes
// parámetros:
//
//  - procs_info: Array que contiene la información de cada proceso activo
//  - procs_count: Cantidad de procesos activos
//  - curr_time: Tiempo actual de la simulación
//  - curr_pid: PID del proceso que se está ejecutando en el CPU
//
// Esta función se ejecuta en cada timer-interrupt donde existan procesos
// activos (se asegura que `procs_count > 0`) y determina el PID del proceso a
// ejecutar. El valor de retorno es un entero que indica el PID de dicho
// proceso. Pueden ocurrir tres casos:
//
//  - La función devuelve -1: No se ejecuta ningún proceso.
//  - La función devuelve un PID igual al curr_pid: Se mantiene en ejecución el
//  proceso actual.
//  - La función devuelve un PID diferente al curr_pid: Simula un cambio de
//  contexto y se ejecuta el proceso indicado.
//


// int *queue;
// int *pid_q;

int queue[MAX_PROCESS_COUNT];
int pid_q[MAX_PROCESS_COUNT];


int time_slice = 100;
int priority_boost = 1000;
int time_in_cpu = 0;

int fifo_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,
                   int curr_pid) {
  // Se devuelve el PID del primer proceso de todos los disponibles (los
  // procesos están ordenados por orden de llegada).
  return procs_info[0].pid;
}

int sjf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {

  int ans_pid = procs_info[0].pid;
  int ans_duration = process_total_time(ans_pid);

  if(curr_pid == -1){

    for (int i = 1; i < procs_count; i++){
      int i_pid = procs_info[i].pid;
      int i_duration = process_total_time(i_pid);

      if(i_duration < ans_duration){
        ans_pid = i_pid;
        ans_duration = i_duration;
      }
    }
  }
  else {
    ans_pid = curr_pid;
  }
  return ans_pid;
}

int stcf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {

  int ans_pid = procs_info[0].pid;
  int exec_time = procs_info[0].executed_time;
  int duration = process_total_time(ans_pid);

  int ans_time_to_completion = duration - exec_time;

  for (int i = 1; i < procs_count; i++){

    int i_pid = procs_info[i].pid;
    int i_exec_time = procs_info[i].executed_time;
    int i_duration = process_total_time(i_pid);
    int i_time_to_completion = i_duration - i_exec_time;

    if(ans_time_to_completion > i_time_to_completion){
      ans_pid = i_pid;
      ans_time_to_completion = i_time_to_completion;
    }
  }
  return ans_pid;
}

int rr_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {
  if(curr_time % time_slice == 0 || curr_pid == -1){
    if(curr_pid == procs_info[procs_count - 1].pid || curr_pid == -1){ return procs_info[0].pid; }
    else{
      for(int i = 0; i < procs_count; i++){
        if(procs_info[i].pid == curr_pid){
          return procs_info[i+1].pid;
        }
      }
      return procs_info[0].pid;
    }
  }
  return curr_pid;
}

int aux_rr_scheduler(int procs_info[], int procs_count, int curr_time,int curr_pid) {
  if(curr_time % time_slice == 0 || curr_pid == -1){
    if(curr_pid == procs_info[procs_count - 1] || curr_pid == -1){ return procs_info[0]; }
    else{
      for(int i = 0; i < procs_count; i++){
        if(procs_info[i] == curr_pid){
          return procs_info[i+1];
        }
      }
      return procs_info[0];
    }
  }
  int f =0;
  for(int i = 0; i < procs_count; i++){
        if(procs_info[i] == curr_pid){
          f=1;
        }}
  if(f==1){return curr_pid;}
  else{return procs_info[0];}
}

static void increase_time_in_cpu(proc_info_t *procs_info,int procs_count, int curr_pid, int curr_time){
  if(curr_pid != -1){
    for(int i=0;i<procs_count;i++){
     if(curr_pid==procs_info[i].pid && procs_info[i].on_io == 0){
       time_in_cpu=time_in_cpu+10;
       break;
      }
    }
  }
}

static void update_queues(proc_info_t *procs_info, int procs_count, int curr_pid,int curr_time){
  int found = 0;
  for(int i = 0;i<procs_count;i++){
    found = 0;
    for(int j =0;j<procs_count;j++){
      if(procs_info[i].pid==pid_q[j]){
        int a = pid_q[i];
        int b = queue[i];
        queue[i] = queue[j];
        pid_q[i] = pid_q[j];
        queue[j] = b;
        pid_q[j] = a;
        found = 1;
      }
    }
    if(found==0){
      queue[i]=0;
      pid_q[i]=procs_info[i].pid;
    }
    if(procs_info[i].pid==curr_pid && queue[i] < 2 && time_in_cpu > 0 && time_in_cpu % time_slice ==0){
      queue[i] = queue[i] +1;
    }
  }
}

int milf_queuer(proc_info_t *procs_info, int procs_count,int curr_pid,int curr_time){
    int queue_0[procs_count];
    int q_0 = 0;
    int queue_1[procs_count];
    int q_1 = 0;
    int queue_2[procs_count];
    int q_2 = 0;
    for(int i = 0; i<procs_count;i++){
      if(procs_info[i].on_io != 1){
        if(queue[i]==0){
          queue_0[q_0] = procs_info[i].pid;
          q_0 = q_0 +1;
        }
        if(queue[i]==1){
          queue_1[q_1] = procs_info[i].pid;
          q_1 = q_1 +1;
        }
        if(queue[i]==2){
          queue_2[q_2] = procs_info[i].pid;
          q_2 = q_2 +1;
        }
      }
    }
    int ans = -1;
    if(q_0 != 0){
      ans= aux_rr_scheduler(queue_0,q_0,time_in_cpu,curr_pid);
      if(curr_time >= 2500 && curr_time<=3100){printf("queue_0 ");printf("%d",q_0);
        printf(" pid ");printf("%d",ans);putchar('\n');}
    }
    else if(q_1 != 0) {ans= aux_rr_scheduler(queue_1,q_1,time_in_cpu,curr_pid);
if(curr_time >= 2500 && curr_time<=3100){ printf("queue_1 ");printf("%d",q_1);printf(" pid ");printf("%d",ans);putchar('\n');}
          }
    else if(q_2 != 0){ans= aux_rr_scheduler(queue_2,q_2,time_in_cpu,curr_pid);
if(curr_time >= 2500 && curr_time<=3100){ printf("queue_2 ");printf("%d",q_2);printf(" pid ");printf("%d",ans);putchar('\n');}
          }
    else{ans= procs_info[0].pid;}

    if(ans != curr_pid){time_in_cpu = 0;}
    return ans;
}

int milf_scheduler(proc_info_t *procs_info, int procs_count, int curr_time,int curr_pid) {

  increase_time_in_cpu(procs_info, procs_count, curr_pid,curr_time);//aumento el tiempo que lleva el proceso en cpu
  update_queues(procs_info,procs_count,curr_pid,curr_time);         //actualizo las colas de prioridad


  if(curr_time % priority_boost == 0){                    // si hay priority_boost pongo todos los procesos en la cola 0
    for(int i = 0; i<procs_count;i++){
      queue[i] = 0;
      time_in_cpu=0;
    }
    return rr_scheduler(procs_info, procs_count, curr_time, curr_pid);
  }

  return milf_queuer(procs_info,procs_count,curr_pid,curr_time);//formar las colas y pasarle la correspondiente a rr_scheduler
}

// Esta función devuelve la función que se ejecutará en cada timer-interrupt
// según el nombre del scheduler.
schedule_action_t get_scheduler(const char *name) {
  // Si necesitas inicializar alguna estructura antes de comenzar la simulación
  // puedes hacerlo aquí.

  for(int i = 0;i<MAX_PROCESS_COUNT;i++){
  pid_q[i] = -1;
  }
  if (strcmp(name, "fifo") == 0) return *fifo_scheduler;

//   sjf_scheduler
  if (strcmp(name, "sjf") == 0) return *sjf_scheduler;

//  stcf_scheduler
  if (strcmp(name, "stcf") == 0) return *stcf_scheduler;

//  rr_scheduler
  if (strcmp(name, "rr") == 0) return *rr_scheduler;

//  milf_scheduler
  if (strcmp(name, "mlfq") == 0){return *milf_scheduler;}

  fprintf(stderr, "Invalid scheduler name: '%s'\n", name);
  exit(1);
}

