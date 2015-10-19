LINUX_C_THREADS

<h2>gcc main.c -o main -std=c99 -lpthread -g && ./main</h2>
<ul>
	<li>-std=c99 - C99-стандарт Си. Флаг был добавлен для использования конструкции for(int i, , );</li>
	<li>-lpthread - Использование потоков в программе</li>
	<li>-g - Включен режим отладки </li>
</ul>

<h2>Отладка gdb</h2>
<ul>
	<li>$ gdb main</li>
	<li>(gdb) run</li>
</ul>
