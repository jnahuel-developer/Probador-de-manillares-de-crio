# 🛠️ Probador de Manillares de Crio  

Dispositivo para probar y validar el correcto funcionamiento de manillares para la terapia de Crio Radio Frecuencia.  
Este repositorio incluye tanto el firmware para el microcontrolador como el diseño del PCB.  


![Vista 3D del PCB](https://github.com/jnahuel-developer/Probador-de-manillares-de-crio/blob/main/Hardware/ProbadorManillaresCrio/Images/ProbadorManillaresCrio%20-%203D.jpg)  


---
---


## ❄️ Manillar para la terapia de Crio Radio Frecuencia  

El manillar para la terapia de Crio Radio Frecuencia combina un sistema de refrigeración con un arreglo de terminales para la emisión de RF.  

### 🔹 **Sistema de Refrigeración**  
El sistema de refrigeración incluye una celda Peltier, encargada de disminuir la temperatura sobre los pernos hasta -15°C. Para disipar el calor generado, cuenta con un circuito cerrado de circulación de agua que lo transporta fuera del manillar.  

Para gestionar este proceso, el **Probador** integra:  
- Circuitos independientes de activación de una **bomba de agua de 12V** y un **cooler** de refrigeración del agua.  
- Un **caudalímetro**, que verifica la correcta circulación del flujo de agua.  
- Un **sensor DS18S10**, ubicado en el ingreso de la bomba, que monitorea la temperatura y previene condiciones inseguras.  
- Un **circuito de alimentación de potencia** para la celda Peltier, con una medición de la corriente consumida mediante una **resistencia Shunt**.  
- **Dos sensores DS18S10** adicionales, que registran la temperatura en ambas caras de la celda.  

### 🔹 **Emisión de RF**  
Para la verificación de la emisión de radiofrecuencia, el **Probador** emplea dos **multiplexores analógicos de 16 canales**, los cuales permiten inyectar señales y medir la continuidad de los circuitos. Esto posibilita no solo verificar que los pernos tengan continuidad, sino también que estén correctamente ubicados.  

### 🔹 **Interfaz y Control**  
El **Probador** se maneja mediante un sistema de **cuatro pulsadores** y una **pantalla LCD**, que guían al operador durante el proceso de prueba del manillar.  

Este sistema permite realizar un test completo de cada unidad de manera rápida y eficiente, asegurando su correcto funcionamiento sin necesidad de recurrir a equipos comerciales.


---
---


## 📁 Contenido del Repositorio  


📂 **Probador-de-manillares-de-crio/**  
├── 📁 **firmware/** → Código fuente para el microcontrolador  
├── 📁 **hardware/** → Diseño del PCB, esquemáticos y archivos Gerber  
├── 📜 **README.md** → Documentación principal  


---


## 🖼️ Modelos 3D diseñados en Altium Designer  


Vista frontal del PCB:  
![PCB real](https://github.com/jnahuel-developer/Probador-de-manillares-de-crio/blob/main/Hardware/ProbadorManillaresCrio/Images/ProbadorManillaresCrio%20-%20Frente.jpg)  


Vista posterior del PCB:  
![PCB real](https://github.com/jnahuel-developer/Probador-de-manillares-de-crio/blob/main/Hardware/ProbadorManillaresCrio/Images/ProbadorManillaresCrio%20-%20Dorso.jpg)  


---


## 🛠️ Herramientas Utilizadas  


- **Diseño PCB** → Altium Designer  
- **Modelado 3D** → Altium Designer  
- **Firmware Target** → MC9S08AC16CFD  
- **Compiler** → CodeWarrior HCS08 C Compiler  
- **Lenguaje** → C  


---


📩 **Contacto:** [jnahuel.developer@gmail.com](jnahuel.developer@gmail.com)  

📩 **Contacto:** [https://www.linkedin.com/in/jnahuel/](https://www.linkedin.com/in/jnahuel/)  
