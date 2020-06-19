# TP_EJ4

En una PyME de desarrollo de software, el área de testing necesita de una herramienta que pueda
mínimamente detectar “fugas” de memoria, así como excesiva utilización de CPU, para esto le
solicita a usted, pasante de la empresa, que implemente dicha herramienta siguiendo con las
especificaciones de diseño que fueron realizadas por el líder del equipo de testing;
1. Codificar un proceso Principal cuya tarea será crear dos procesos hijos (Control y Registro),
los tres procesos deberán quedar ejecutando en segundo plano. Una vez creados ambos
procesos hijos, el proceso Principal deberá quedar a la espera de la señal SIGUSR1, cuando
se reciba dicha señal los tres procesos deberán finalizar.
2. El proceso Control deberá detectar (cada un segundo) si algún proceso en ejecución supera
un valor límite de consumo de memoria o CPU, si dicha situación ocurre deberá enviar al
proceso Registro un conjunto de datos a través de un FIFO.
Los datos a enviar son:
• PID
• Nombre
• Tipo de exceso (Memoria – CPU – Ambos)
• Hora del sistema (HH:MM: SS)
Los valores límite deben ser pasados como parámetros al proceso Principal.
1. El proceso Registro recibirá los datos y los registrará en un archivo (una línea por cada
proceso), cabe destacar que solo se registrará el mismo proceso a los sumo dos veces,
por ejemplo;
${PID}: Supera CPU (Primer registro)
${PID}: Supera memoria o Ambos (Segundo registro)
ó
${PID}: Supera memoria (Primer registro)
${PID}: Supera CPU o Ambos (Segundo registro)
(Nota el campo ${PID} deberá ser reemplazado por el PID del proceso que haya generado el
evento)
Solo se registrará una vez aquellos procesos donde el primer registro sea por exceso de tipo
Ambos o que solo excedan siempre CPU y no memoria o viceversa. 
