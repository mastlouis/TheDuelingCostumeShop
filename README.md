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

*************************************************
The Costume Shop: Summary of Program logic
*************************************************

The following is an explanation of the logic behind the program. If this is too long to read, please scroll down to the notes.

The program starts by checking for valid arguments. For the program to run properly, arguments must be entered in the following format: ./part1 <number of teams> <number of pirates> <number of ninjas> <average pirate costuming time> <average ninja costuming time> <average pirate arrival time> <average ninja arrival time>. The number of teams must be between 2 and 4 (inclusive), and the number of pirates and the number of ninjas must each be between 10 and 50 (inclusive). The average costuming time of the pirates is intended to be higher than that of the ninjas, but nothing enforces this because enforcement was not specified in the assignment instructions.

The program then initializes a massive struct to keep track of all the data in the shop, the structs to track the data for each thread, and the threads themselves. The main function then uses pthread_join() to wait for each thread while they execute the costumeDept() function, which is where most of the program takes place.

Each thread enters the costumeDept() function with a struct of data containing an array of visits (structs to record data for each visit), information about whether the thread is a pirate or a ninja, some statistics, and a pointer to the costume shop data struct. The pointer to the costume shop data struct is saved on each thread's stack for simplicity.

Most of the function is in an outer loop through which each thread will iterate until it decides not to return to the costume shop. On each iteration of this outer loop, the thread will sleep a random amount of time to simulate adventuring before coming to the shop. Next, the thread repeatedly checks whether or not it can enter the shop, sleeping between checks. For a thread to be eligible to enter the shop, three conditions must be true: there must be at least one available team; there must be no members of the opposing faction; and the thread's faction cannot be blocked. A thread must acquire the lock that controls all of the shop's doors to check its eligibility. This checking, entering, and acquiring a team are one atomic, locked transaction.

While entering, a thread also checks whether or not it should block threads of the other faction (a power which in the spirit of fairness is given to each thread). If any given thread has been waiting for longer than a critical threshold of seconds, it prevents the other team from entering. This critical threshold is 30 (the amount of time a thread can wait before costuming is free) minus the average wait time for the other side. This mechanism is discussed more in problem1_explanation.txt.

Once a thread enters the shop, it sleeps for a random amount of time according to its faction representing the amount of time for which the tread is being costumed. When this is done, the thread must re-acquire the door lock to exit the shop. To exit is to decrement the count of threads of that faction in the shop, to relinquish the team the thread was using, and to record some data about the visit. Exiting occurs atomically via locks.

After each visit, a thread records some statistics about the visit, including the amount of time it spent in the shop and the amount of time the thread spent waiting. A thread then decides whether or not it would like to come back to the costume shop using a pseudorandomly generated number. If the thread wants to return to the shop, it will iterate through the outer loop again with some values reset. Otherwise, it will exit. This allows each thread to join with the main thread.

After all threads have joined the main thread, the main thread will print the assignment-requested statistics using a function call, then free the allocated data. The main thread then frees all remaining allocated data, then exits.

*************************************************
The Costume Shop: Defects
*************************************************

One aspect in which this implementation of the costume shop is suboptimal is the fairness strategy. The following explanation is reprinted from problem1_explanation.txt:

In order to make the costume department more fair, we implemented a system wherein pirates and ninjas have the ability to block the other faction if they've been waiting for a certain amount of time. This amount of time is thirty minutes (the amount of time after which a pirate or ninja will receive their costume for free) minus one minute longer than the average time taken to costume someone of the other faction. The hope behind this is that there will be just enough time for the shop to empty out and let in people who are waiting before they would get their costumes for free.

The problem with this solution in its simplest form is what to do when the shop gets so backed up that some pirates and ninjas have been waiting for so long that they should each be blocking the other. To address this problem, we added a scaling factor on each faction's critical time (the time after which they should block the other side), and these scaling factors get incremented with each successive block until one line (either the line of pirates or the line of ninjas) becomes empty, at which point each scaling factor will be reset.

This solution attempts to maximize profit by attempting to let in as many people as possible before they can get costumed for free. The solution will not always be effective because the wait times are unpredictable and pseudorandomly generated. Another weakness of the solution is that if one side has been waiting for a very long time, the other side must also wait a very long time. This can be frustrating for the people involved, and the shop will likely lose a lot of money to customers who could have been let in but were blocked, but we would argue that this is this is a fair way to run the shop that allows both factions access to the shop.

If the shop only sought to make money, the strategy would be to let in someone who's been waiting longest with a maximum of 29 minutes and to leave anyone who had been waiting for more than 30 minutes until the end. Serving customers who will get their cosumes for free only holds up potential paying customers.

*************************************************
The Costume Shop: Additional Notes
*************************************************

Some pirates and ninjas have unique names, but many have a default name. The default name for ninjas is Ninja Ninjington, and the default name for pirates is Pirate McPirateson.

