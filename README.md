# Concurrency

Created a simuation to practice critical region and multithreading by simulating a teacher office hour.

There is a class A and a class B and the teachers office has only 3 seats.

The teachers office is the critical region and I must ensure 

1). only students from one class can be in the office at a time.

2). after 10 students the teacher take a break and if there is a student in there he waits until he is done.

3). after 5 consecutive students of a class it must switch over to the other class.

3). If there are students waiting in outside the office and there is no students in the office then they should be able to enter.
