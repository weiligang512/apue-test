#include <stdio.h>

void printLingXing (int n);

int
main (void)
{
  int n;
  printf ("Enter a integer odd number: ");
  scanf ("%d", &n);

  printLingXing (n);

  return 0;
}

void
printLingXing (int n)
{
  int i, j;
  int k;			//记录'*'在的每一行的第一个位置
  for (i = 0; i < n; ++i)
    {
      k = n / 2 - i;		//每行第一个‘*’号的位置,当k为负的时候是菱形的下部分，其绝对值也是该行第一个‘*’号的位置
      for (j = 0; j < n; ++j)
	{
	  if (k >= 0)		//菱形的上一部分
	    {
	      if (j < k || j > n / 2 + i)	//条件：j < k 是第一个'*'前,j > n / 2 + i 是最后一个'*'后
		printf ("a");
	      else		//在每一行的第一个‘*’和最后一个‘*’号之间,交替打印‘*’和‘c’
		{
		  if ((i + j) % 2 != 0)	//根据菱形的形状，‘*’出现在i+j的奇数位置
		    printf ("*");
		  else
		    printf ("c");
		}
	    }
	  else if (k < 0)	//菱形的下一部分
	    {
	      if ((j < -k) || (j > n - 1 + k))	//条件：j < -k 是第一个'*'前,j > n - 1 + k 是最后一个'*'后
		printf ("A");
	      else		//在每一行的第一个‘*’和最后一个‘*’号之间,交替打印‘*’和‘c’
		{
		  if ((i + j) % 2 != 0)
		    printf ("*");
		  else
		    printf ("c");
		}
	    }
	}
      printf ("\n");
    }
}
