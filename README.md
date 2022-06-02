# Order Matching Engine
<img src=pic.gif width="500" height="200"/>

A system that matches `BUY` and `SELL` orders. Users can submit several types of orders to this engine, which will process the submitted orders and place them into an order book.

`orderEngine.h` is the header file and contains the relevant headers for this program.

`orderEngine.cpp` is the cpp file that implements the all the operations of this order matching system.

`main.cpp` parses the user input and sends it to the order matching engine to process to shoot out an output.

Orders in the order book are represented by `[Quantity]@[Price]#[OrderID]`. E.g. a BUY order represented as `350@9#qu82` is an order where the user is willing to buy 350 units of the stock at $9.

An example of a user input of a LIMIT BUY order with a quantity of 300 and a price of 9 and an orderID of 'Fu78'

```
SUB LO B Fu78 300 9
```

## Types of Orders:

#### 1. LIMIT (LO)
#### 2. MARKET (MO)
#### 3. IMMEDIATE-OR-CANCEL (IOC)
#### 4. FILL-OR-KILL (FOK)
#### 5. CANCEL (CXL)
#### 6. CANCEL/REPLACE (CRP)
#### 7. ICEBERG (ICE)

## How to Run
Download repository into a folder, open terminal in the directory and run `make`. This will compile the code.

## TEST 1: Testing LO, MO, CXL orders
```
./a.out < test1.in
```
<table>
<tr>
<th>INPUT</th>
<th>OUTPUT</th>
</tr>
<tr>
<td>
<pre>
SUB LO B Ffuj 200 13
SUB LO B Yy7P 150 11
SUB LO B YuFU 100 13
SUB LO S IpD8 150 14
SUB LO S y93N 190 15
SUB LO B Y5wb 230 14
SUB MO B IZLO 250
CXL Ffuj
CXL 49Ze
END
</pre>
</td>
<td>

```json
0
0
0
0
0
2100
2850
B: 80@14#Y5wb 100@13#YuFU 150@11#Yy7P
S:
```
</td>
</tr>
</table>

## TEST 2: Testing CRP, FOK, IOC, ICE ORDERS
```
./a.out < test2.in
```
<table>
<tr>
<th>INPUT</th>
<th>OUTPUT</th>
</tr>
<tr>
<td>
<pre>
SUB LO B N1Eh 300 12
SUB LO B 0Gxb 250 11
SUB LO S JSvU 350 14
SUB LO S uH6w 320 15
SUB IOC S ckMR 150 10
SUB IOC B DVeP 500 14
SUB FOK S ejnR 200 12
SUB FOK S 8uGs 200 9
SUB LO B 2va9 250 12
SUB LO B 9zS1 300 11
CRP 2va9 480 11
CRP 9zS1 170 11
SUB ICE S 9SDF 300 10 50
SUB ICE B y8s9 600 15 100
SUB LO S uy82 170 11
END
</pre>
</td>
<td>

```json
0
0
0
0
1800
4900
0
2350
0
0
3300
4800
1100
B: 100(110)@15#y8s9 70@11#9zS1 480@11#2va9 
S: 
```
</td>
</tr>
</table>


**_Adapted from AlphaLabCapital Test_**

