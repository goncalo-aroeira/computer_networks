# Network Data Names (NDN) Project

## 📌 **Project Description**
This project implements a distributed **Named Data Network (NDN)** where nodes communicate using TCP and UDP. Nodes dynamically join the network, assign external and internal neighbors, and exchange data using named objects.

---

## ⚙️ **Compilation**
To compile the project, run the following command:
```bash
make
```
This will generate the executable file **ndn_node**.

To clean up compiled files, use:
```bash
make clean
```

---

## 🚀 **Running the Program**
Each node requires an **IP address** and a **port number**. The general syntax for running a node is:
```bash
./ndn_node <IP> <PORT>
```
Example:
```bash
./ndn_node 172.30.85.152 5000
```

---

## 📡 **Example: Running 3 Nodes**
### **1️⃣ Start Node 1 (5000)**
```bash
./ndn_node 172.30.85.152 5000
```
Inside the node, enter:
```bash
join 001
show_topology
```
Expected output:
```
External neighbor: -1
Safeguard: -1
Internal neighbors:
```

### **2️⃣ Start Node 2 (5001)**
```bash
./ndn_node 172.30.85.152 5001
```
Inside the node, enter:
```bash
direct_join 001 172.30.85.152 5000
show_topology
```
Expected output:
```
External neighbor: 172.30.85.152:5000
Safeguard: -1
Internal neighbors:
```
On Node 5000, enter:
```bash
show_topology
```
Expected output:
```
External neighbor: 172.30.85.152:5001
Safeguard: -1
Internal neighbors:
```

### **3️⃣ Start Node 3 (5002)**
```bash
./ndn_node 172.30.85.152 5002
```
Inside the node, enter:
```bash
direct_join 001 172.30.85.152 5001
show_topology
```
Expected output:
```
External neighbor: 172.30.85.152:5001
Safeguard: -1
Internal neighbors:
```
On Node 5001, enter:
```bash
show_topology
```
Expected output:
```
External neighbor: 172.30.85.152:5000
Safeguard: -1
Internal neighbors: 172.30.85.152:5002
```
On Node 5000, enter:
```bash
show_topology
```
Expected output:
```
External neighbor: 172.30.85.152:5001
Safeguard: -1
Internal neighbors:
```

---

## 🖥 **Available Commands**
| **Command** | **Description** |
|------------|---------------|
| `join net` | Joins a network (e.g., `join 001`). |
| `direct_join net IP TCP` | Directly connects to another node (e.g., `direct_join 001 172.30.85.152 5000`). |
| `show_topology` | Displays the external neighbor, safeguard, and internal neighbors. |
| `exit` | Closes the node. |
