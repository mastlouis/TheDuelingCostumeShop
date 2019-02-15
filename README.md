# TheDuelingCostumeShop

Questions for the TA
-Can we treat the teams as indestinguishable?
-Do MA drivers always enter the intersection if then can?
	-How do we avoid deadlocks?
-How "Itemized" is an Itemized receipt?



Matthew's Ideas (Just to gather them into one place):

-If we measure time in seconds, and each second is meant to be indivisible,
 then we should have each person sleep for 1 second every time they fail to 
 get the lock.
	-Each person should keep track of the length of time for which they've 
	 waited in a stack variable. This makes time relative, which is weird, 
	but should ultimately be okay if things move much faster than seconds.

-We can keep a variable in the struct tracking how many pirates and ninjas 
 are currently waiting to get into the shop. This can help the shop decide 
 whether or not it needs switch from pirates to ninjas.

	-A pirate or ninja can tell whether or not it needs to remove itself 
	from this count by looking at the amount of time it's waited (whether 
	or not it's zero).

-We should have a variable in the struct to track the number of available 
teams of costumers.

**********************************
Part 1: The Costume Shop
**********************************

The following is an explanation of the logic behind the program.

The program starts by checking for valid arguments. For the program to run properly, arguments must be entered in the following format: ./part1 <number of teams> <number of pirates> <number of ninjas> <average pirate costuming time> <average ninja costuming time> <average pirate arrival time> <average ninja arrival time>. The number of teams must be between 2 and 4 (inclusive), and the number of pirates and the number of ninjas must each be between 10 and 50 (inclusive). The average costuming time of the pirates is intended to be higher than that of the ninjas, but nothing enforces this because enforcement was not specified in the assignment instructions.

The program then initializes a massive struct to keep track of all the data in the shop, the structs to track the data for each thread, and the threads themselves. The main function then uses pthread_join() to wait for each thread while they execute the costumeDept() function, which is where most of the program takes place.

Each thread enters the costumeDept() function with a struct of data containing an array of visits (structs to record data for each visit), information about whether the thread is a pirate or a ninja, some statistics, and a pointer to the costume shop data struct. The pointer to the costume shop data struct is saved on each thread's stack for simplicity.

Most of the function is in an outer loop through which each thread will iterate until it decides not to return to the costume shop. On each iteration of this outer loop, the thread will sleep a random amount of time to simulate adventuring before coming to the shop. Next, the thread repeatedly checks whether or not it can enter the shop, sleeping between checks. For a thread to be eligible to enter the shop, three conditions must be true: there must be at least one available team; there must be no members of the opposing faction; and the thread's faction cannot be blocked. A thread must acquire the lock that controls all of the shop's doors to check its eligibility. This checking, entering, and acquiring a team are one atomic, locked transaction.

Once a thread enters the shop, it sleeps for a random amount of time according to its faction representing the amount of time for which the tread is being costumed. When this is done, the thread must re-acquire the door lock to exit the shop. To exit is to decrement the count of threads of that faction in the shop, to relinquish the team the thread was using, and to record some data about the visit. Exiting occurs atomically via locks.



