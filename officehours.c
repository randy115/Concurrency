/*
 Name: Randy Bui
 ID: 1001338008
 */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1
#define CLASSC 2
#define CLASSD 3

/* TODO */
/* Add your synchronization variables here */

/* Basic information about simulation.  They are printed/checked at the end
 * and in assert statements during execution.
 *
 * You are responsible for maintaining the integrity of these variables in the
 * code that you develop.
 */

sem_t mutex;
pthread_mutex_t lockenter;
pthread_mutex_t lockleave;
pthread_mutex_t classprofbreak;
static int consecutive_a;            //to see how many students of class a entered consecutively
static int consecutive_b;            //to see how many students of class b entered consecutively
static int students_in_office;       /* Total numbers of students currently in the office */
static int classa_inoffice;          /* Total numbers of students from class A currently in the office */
static int classb_inoffice;          /* Total numbers of students from class B in the office */
static int students_since_break = 0; //to see how many students entered office since break
static int wait_classa = 0;          //to see how many students of class a are waiting outside office
static int wait_classb = 0;          //to see how many students of class b are waiting outside office
typedef struct
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;
  int class;
} student_info;

/* Called at beginning of simulation.
 * TODO: Create/initialize all synchronization
 * variables and other global variables that you add.
 */
static int initialize(student_info *si, char *filename)
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;
  wait_classa = 0;
  wait_classb = 0;
  consecutive_a = 0;
  consecutive_b = 0;
  //initializing mutexes
  pthread_mutex_init(&lockenter, NULL);
  pthread_mutex_init(&lockleave, NULL);
  pthread_mutex_init(&classprofbreak, NULL);
  sem_init(&mutex, 0, 3); //initializing semaphore where it only allows 3 threads in critical region

  /* Initialize your synchronization variables (and
   * other variables you might use) here
   */

  /* Read in the data file and initialize the student array */
  FILE *fp;

  if ((fp = fopen(filename, "r")) == NULL)
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ((fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time)) != EOF) &&
         i < MAX_STUDENTS)
  {
    i++;
  }

  fclose(fp);
  return i;
}

/* Code executed by professor to simulate taking a break
 * You do not need to add anything here.
 */
static void take_break()
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  assert(students_in_office == 0);
  students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 */
void *professorthread(void *junk)
{
  printf("The professor arrived and is starting his office hours\n");

  /* Loop while waiting for students to arrive. */
  while (1)
  {
    //check if 10 or more students since break
    if (students_since_break == 10)
    {
      //set boolean to true to signal that professor is taking break
      //while loop make sures to wait until all students
      //are out of the office before the professor takes a break
      while (students_in_office != 0)
      {
      }
      //lock take a break function in mutex so no other
      //classes can try to come in while the professor
      //is taking break also set boolean professtakebreak
      //to false because he is not taking a break anymore
      pthread_mutex_lock(&classprofbreak);
      take_break();
      pthread_mutex_unlock(&classprofbreak);
    }
  }
  pthread_exit(NULL);
}

/* Code executed by a class A student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classa_enter()
{
  //Check if 5 students from class a have consecutively gone in
  //Check if a student from class b is already in the officehour

  //increment the number of students from class A waiting outside of office

  //lock the thread to make sure one student goes in at a time
  pthread_mutex_lock(&lockenter);
  wait_classa++;
  //while loop conditions which ensure class A
  //students have to wait because
  //1).5 consecutive student A or greater
  //2).class B students are in office
  //3).the students that have enter the office since the last break is greater than 10
  while ((consecutive_a >= 5) || (classb_inoffice > 0) || (students_since_break >= 10))
  {
    //mutex unlocks here to let another thread grab lock
    pthread_mutex_unlock(&lockenter);
    //another thread grabbing the lock
    pthread_mutex_lock(&lockenter);
    //this if statement is a condition to let
    //class A students
    //while waiting in line into office
    //1).no students from class B are in the office
    //2).students since the professor took a break is less than 10
    //3).there are no students from class B waiting outside the office
    if ((classb_inoffice == 0) && (students_since_break < 10) && (wait_classb == 0))
    {
      break;
    }
    //usleep(1);
  }
  //start of critical region, allows up to 3 threads to enter at once
  sem_wait(&mutex);
  wait_classa--;
  students_in_office += 1;
  students_since_break += 1;
  consecutive_a++;
  consecutive_b = 0;
  classa_inoffice += 1;
  pthread_mutex_unlock(&lockenter);
  //unlock mutex to give class B a chance to run
}

/* Code executed by a class B student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classb_enter()
{

  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE.                                                     */
  //increment the number of students from class B waiting outside of office
  //lock the thread to make sure one student goes in at a time
  pthread_mutex_lock(&lockenter);
  wait_classb++;
  //while loop conditions which ensure class B
  //students have to wait because
  //1).5 consecutive student B or greater
  //2).class A students are in office
  //3).the students that have enter the office since the last break is greater than 10
  while ((consecutive_b >= 5) || (classa_inoffice > 0) || (students_since_break >= 10))
  {
    //mutex unlocks here to let another thread grab lock
    pthread_mutex_unlock(&lockenter);
    //another thread grabbing the lock
    pthread_mutex_lock(&lockenter);
    //this if statement is a condition to let
    //class B students
    //while waiting in line into office
    //1).no students from class A are in the office
    //2).students since the professor took a break is less than 10
    //3).there are no students from class A waiting outside the office
    if ((classa_inoffice == 0) && (students_since_break < 10) && (wait_classa == 0))
    {
      break;
    }
    //usleep(1);
  }
  //critical region where only 3 threads can enter at a time
  sem_wait(&mutex);
  wait_classb--;
  consecutive_b++;
  consecutive_a = 0;
  students_in_office += 1;
  students_since_break += 1;
  classb_inoffice += 1;
  pthread_mutex_unlock(&lockenter);
  //unlock the mutex so class A has a chance to grab it
}

/* Code executed by a student to simulate the time he spends in the office asking questions
 * You do not need to add anything here.
 */
static void ask_questions(int t)
{
  sleep(t);
}

/* Code executed by a class A student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classa_leave()
{
  /*
   *  TODO
   *  YOUR CODE HERE.
   */
  //mutex lock to insure that student from class A leave one at a time
  pthread_mutex_lock(&lockleave);
  students_in_office -= 1;
  classa_inoffice -= 1;
  //sem_post is when class A thread leaves critical region
  //and allows another thread to come in
  sem_post(&mutex);
  pthread_mutex_unlock(&lockleave);
  //mutex unlock allows another student to leave office
}

/* Code executed by a class B student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classb_leave()
{
  /*
   * TODO
   * YOUR CODE HERE.
   */
  //mutex lock to insure student from class B leave one at a time
  pthread_mutex_lock(&lockleave);
  students_in_office -= 1;
  classb_inoffice -= 1;
  //sem_post is when class B thread leaves critical region
  //and allowd another thread to come in
  sem_post(&mutex);
  pthread_mutex_unlock(&lockleave);
  //mutex unlock allows another student to leave office
}

/* Main code for class A student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void *classa_student(void *si)
{
  student_info *s_info = (student_info *)si;

  /* enter office */
  classa_enter();

  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0);

  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classa_leave();

  printf("Student %d from class A leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void *classb_student(void *si)
{
  student_info *s_info = (student_info *)si;

  /* enter office */
  classb_enter();

  printf("Student %d from class B enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0);

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classb_leave();

  printf("Student %d from class B leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 */
int main(int nargs, char **args)
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2)
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0)
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
         num_students);

  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result)
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i = 0; i < num_students; i++)
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);

    student_type = random() % 2;

    if (s_info[i].class == CLASSA)
    {
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
    }
    else // student_type == CLASSB
    {
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
    }

    if (result)
    {
      printf("officehour: thread_fork failed for student %d: %s\n",
             i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++)
  {
    pthread_join(student_tid[i], &status);
  }

  /* tell the professor to finish. */
  pthread_cancel(professor_tid);

  printf("Office hour simulation done.\n");

  return 0;
}
