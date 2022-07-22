# nutrition_calculator
This util helps you to save time on calculating daily consumption of calories, protein, fat and carbohydrates. You only type a product title and its amount you have consumed. If you enter a product first time then the program will ask you to enter its nutrition facts in format 'Protein/Fat/Carbohydrates/Fiber Calories' and then it will be stored for the future usage without asking you the entered info again. When you finish entering your list then using 'finish' command or at any given momemt using 'total' command the summary info in the form of a table will be presented to you where you can see all the calculated values and estimate your current consumption level

    Mikes-MacBook-Pro:nutrition_calculator mike$ nutrition_calculator 
    Nutrition Calculator
    Enter your login name: Mike
    Enter password: 

    Hello, Mike!

    Download remote version of resource '"item_info.txt"'...
    Local resource is up to date: '/var/folders/x4/s5ve23gs4g132nd81mblxewhc40000gn/K/item_info.txt'
    Download remote version of resource '"input.txt"'...
    Local resource is up to date: '/var/folders/x4/s5ve23gs4g132nd81mblxewhc40000gn/K/input.txt'

    New item title (or type 'exit' to finish): [from file]: potatoes
    Item info 'potatoes' found: Protein: 2	Fat: 0.1	Carbs: 14	Fiber: 2.2	Cal: 168
    	grams: 148
    Protein: 2.96	Fat: 0.148	Carbs: 20.72	Fiber: 3.256	Cal: 248.64
    New item title (or type 'exit' to finish): avocado
    Item info 'avocado' found: Protein: 2	Fat: 15	Carbs: 2	Fiber: 7	Cal: 160
    	grams: 34
    Protein: 0.68	Fat: 5.1	Carbs: 0.68	Fiber: 2.38	Cal: 54.4
    New item title (or type 'exit' to finish): bread
    Info of item 'bread' not found. Would you like to create it? (y/n)
    y
    	p/f/c/fib per 100g: 9/3.2/46.3/2.7
    	Calories: 265
    	grams: 30
    Protein: 2.7	Fat: 0.96	Carbs: 13.89	Fiber: 0.81	Cal: 79.5
    New item title (or type 'exit' to finish): cheese   
    Item info 'cheese' found: Protein: 26	Fat: 26	Carbs: 0	Fiber: 0	Cal: 340
    	grams: 24
    Protein: 6.24	Fat: 6.24	Carbs: 0	Fiber: 0	Cal: 81.6
    New item title (or type 'exit' to finish): total
                               title |   w (g) |   p (g) |   f (g) |   c (g) | fib (g) |     cal |
                            potatoes |     148 |    2.96 |   0.148 |   20.72 |   3.256 |  248.64 |
                               bread |      30 |     2.7 |    0.96 |   13.89 |    0.81 |    79.5 |
                              cheese |      24 |    6.24 |    6.24 |       0 |       0 |    81.6 |
                             avocado |      34 |    0.68 |     5.1 |    0.68 |    2.38 |    54.4 |
                              Total: |     236 |   12.58 |  12.448 |   35.29 |   6.446 |  464.14 |
    New item title (or type 'exit' to finish): exit
    You have changes in '"/var/folders/x4/s5ve23gs4g132nd81mblxewhc40000gn/K/item_info.txt"'.
    Would you like to upload your file to the remote? (y/n)
    y
    Uploaded '"/var/folders/x4/s5ve23gs4g132nd81mblxewhc40000gn/K/item_info.txt"'
    You have changes in '"/var/folders/x4/s5ve23gs4g132nd81mblxewhc40000gn/K/input.txt"'.
    Would you like to upload your file to the remote? (y/n)
    y
    Uploaded '"/var/folders/x4/s5ve23gs4g132nd81mblxewhc40000gn/K/input.txt"'
