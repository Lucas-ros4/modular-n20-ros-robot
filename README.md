# Robô com motores n20 e odometria (Em Desenvolvimento) 

Plataforma robótica móvel em fase ativa de desenvolvimento, baseada em modificações mecânicas de um projeto existente e equipada com motores N20 com encoder para estimativa de odometria.

---
<img width="1600" height="1200" alt="n20Robot01" src="https://github.com/user-attachments/assets/9e699624-d2ca-4201-847f-2590444d11a8" />
<img width="1200" height="1600" alt="n20Robot02" src="https://github.com/user-attachments/assets/86a66543-2d71-4cb7-bba5-93c6b297a26b" />
<img width="1600" height="1200" alt="n20Robot03" src="https://github.com/user-attachments/assets/1bf6980b-37be-415c-a39b-3022b33f39c3" />



## Modelo 3D & Modificações Mecânicas

O projeto utiliza como base um modelo 3D existente com as seguintes modificações estruturais:

* **Rolamento Integrado:** Adição de um rolamento em uma das engrenagens para redução de atrito e estabilidade de rotação.
* **Motores N20:** Modificação da engrenagem global para acoplamento e acionamento por motores DC N20 com caixa de redução.
* **Alimentação 2S2P (4x 18650):** Criação de uma *case* dedicada para acomodar 4 baterias Li-Ion 18650 ligadas a um BMS 2S2P.

**Modelo 3D no Thingiverse:** [Thingiverse - Model 7396527](https://www.thingiverse.com/thing:7396527)

---


## Hardware & Controle Atual

Atualmente, o robô opera via controle remoto pelo smartphone via Bluetooth, gerenciado por um firmware ESP32 + driver de ponte H TB6612FNG.

* **Microcontrolador:** ESP32[cite: 1]
* **Driver de Motor:** TB6612FNG[cite: 1]
* **Atuadores:** 2x Motores N20 com encoder incremental[cite: 1]
* **Controle:** Aplicativo *BT Car Controller* ou via comandos no Monitor Serial (USB)[cite: 1]
* **Alimentação:** 4x Baterias 18650 (2S2P) com BMS integrado

---


O objetivo central do projeto é evoluir de uma plataforma teleoperada para um robô autônomo e modular integrável ao ecossistema **ROS (Robot Operating System)**.

- [x] Modificação e impressão mecânica do chassis
- [x] Controle via Bluetooth e leitura básica de encoders[cite: 1]
- [ ] **Integração ROS:** Leitura e publicação dos dados de odometria em tempo real via encoders
- [ ] **Expansão Modular:** Suporte a acoplamento de braços e garras robóticas
- [ ] **Sensoriamento:** Adição de sensores de distância e visão computacional
