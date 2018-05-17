/*
    Saif Mahmud
*/

import java.io.*;
import java.net.*;
import java.util.ArrayList;
import java.util.HashMap;

import static java.lang.System.exit;

class IntegerObject {
    int value;

    IntegerObject(int value) {
        this.value = value;
    }
}

/*
    Message Format :
    ---------------------------------------------------------
    | Source Node | Destination Node | Next Hop | Path Cost |
    ---------------------------------------------------------
 */

class messageFormat implements Serializable {//serializing object to pass through UDP socket
    char sourceNode;
    char destNode;
    private char nextHop;
    double pathCost;

    public messageFormat(char sourceNode, char destNode, char nextHop, double pathCost) {
        this.sourceNode = sourceNode;
        this.destNode = destNode;
        this.nextHop = nextHop;
        this.pathCost = pathCost;
    }

    public String toString() {//print shortest paths
        return "shortest path to node " + destNode + ": the next hop is " + nextHop + " and the cost is " + pathCost;
    }
}


// Every 5 sec : Sender sends DV table

class Sender extends Thread {
    private DatagramSocket datagramSocket;//UDP
    private char sourceNode;
    private ArrayList<messageFormat> distanceVector;

    private HashMap<Character, Integer> neighbours = new HashMap<>();

    public Sender(DatagramSocket datagramSocket, char sourceNode, ArrayList<messageFormat> distanceVector) {
        this.datagramSocket = datagramSocket;
        this.sourceNode = sourceNode;
        this.distanceVector = distanceVector;
    }

    void updateDV(String messageEntry[]) {
        char destNode = messageEntry[0].charAt(0);
        distanceVector.set(destNode - 65, new messageFormat(sourceNode, destNode, destNode, Double.parseDouble(messageEntry[1])));
        neighbours.put(destNode, Integer.parseInt(messageEntry[2]));
    }

    public void run() {
        while (true) {
            for (char node : neighbours.keySet()) {
                try {
                    InetAddress inetAddress = InetAddress.getByName("127.0.0.1");//localhost

                    ByteArrayOutputStream arrayOutputStream = new ByteArrayOutputStream();
                    ObjectOutputStream objectOutputStream = new ObjectOutputStream(arrayOutputStream);
                    objectOutputStream.writeObject(distanceVector);

                    byte[] byteArray = arrayOutputStream.toByteArray();
                    DatagramPacket outgoingPacket = new DatagramPacket(byteArray, byteArray.length, inetAddress, neighbours.get(node));
                    datagramSocket.send(outgoingPacket);

                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            try {
                sleep(5000);//5 sec interval
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}

//Receiver is always in listening state

class Receiver extends Thread {
    private static int INF = 1000000000;

    private DatagramSocket datagramSocket;//UDP
    private char sourceNode;
    private double pathCost[];
    private IntegerObject distanceVectorNotChanged;
    private ArrayList<messageFormat> distanceVector;


    public Receiver(DatagramSocket datagramSocket, ArrayList<messageFormat> distanceVector, double pathCost[], char sourceNode, IntegerObject distanceVectorNotChanged) {
        this.datagramSocket = datagramSocket;
        this.sourceNode = sourceNode;
        this.pathCost = pathCost;
        this.distanceVectorNotChanged = distanceVectorNotChanged;
        this.distanceVector = distanceVector;
    }

    public void run() {
        boolean isPrinted = false;//check stability

        while (true) {
            byte[] message = new byte[1024];
            DatagramPacket incomingPacket = new DatagramPacket(message, 1024);

            try {
                datagramSocket.receive(incomingPacket);
                byte[] incomingPacketData = incomingPacket.getData();

                ByteArrayInputStream arrayInputStream = new ByteArrayInputStream(incomingPacketData);
                ObjectInputStream objectInputStream = new ObjectInputStream(arrayInputStream);

                ArrayList<messageFormat> neighbourDV = (ArrayList<messageFormat>) objectInputStream.readObject();

                boolean flag = true;//checking change in DV

                for (int i = 0; i < distanceVector.size(); i++) {
                    double dist = neighbourDV.get(i).pathCost;
                    char neighbour = neighbourDV.get(i).sourceNode;
                    char dest = neighbourDV.get(i).destNode;

                    if (pathCost[neighbour - 65] + dist < distanceVector.get(dest - 65).pathCost) {// Bellman-Ford Equation
                        distanceVector.set(dest - 65, new messageFormat(sourceNode, dest, neighbour, pathCost[neighbour - 65] + dist));
                        flag = false;
                    }
                }

                if (flag) {
                    distanceVectorNotChanged.value++;
                } else {
                    distanceVectorNotChanged.value = 0;
                }

                if (distanceVectorNotChanged.value >= 30 && !isPrinted) {
                    for (int i = 0; i < distanceVector.size(); i++) {
                        if (distanceVector.get(i).pathCost == INF)
                            continue;
                        System.out.println(distanceVector.get(i));
                    }
                    isPrinted = true;
                    distanceVectorNotChanged.value = 0;
                }

            } catch (IOException | ClassNotFoundException e) {
                e.printStackTrace();
            }
        }
    }
}

public class dv_routing {
    private static int INF = 1000000000;

    private static int port;
    private static char sourceNode;

    public static void main(String[] args) throws IOException, InterruptedException {

        System.out.println("\t=================================");
        System.out.println("\tDistance Vector Routing Algorithm");
        System.out.println("\t=================================");

        System.out.println("\tNode : " + args[0]);
        System.out.println("\tPort : " + args[1]);
        System.out.println("\t---------------------------------\n");


        IntegerObject distanceVectorNotChanged = new IntegerObject(0);
        port = Integer.parseInt(args[1]);
        sourceNode = args[0].charAt(0);

        BufferedReader configFile = new BufferedReader(new FileReader(args[2]));
        int configData = Integer.parseInt(configFile.readLine());
        DatagramSocket datagramSocket = new DatagramSocket(port);//UDP

        ArrayList<messageFormat> distanceVector = new ArrayList<>();
        double pathCost[] = new double[26];

        //Initialization
        for (int i = 0; i < 26; i++) {
            pathCost[i] = INF;
            distanceVector.add(new messageFormat(sourceNode, (char) (i + 65), (char) (i + 65), (i + 65 == sourceNode) ? 0 : 1000000000));
        }


        Sender senderThread = new Sender(datagramSocket, args[0].charAt(0), distanceVector);
        Receiver receiverThread = new Receiver(datagramSocket, distanceVector, pathCost, args[0].charAt(0), distanceVectorNotChanged);

        for (int i = 0; i < configData; i++) {
            String inputLine[] = configFile.readLine().split(" ");
            senderThread.updateDV(inputLine);
            char neighbour = inputLine[0].charAt(0);
            pathCost[neighbour - 65] = Double.parseDouble(inputLine[1]);
        }

        //Thread invoked
        receiverThread.start();
        senderThread.start();

        receiverThread.join();
        senderThread.join();

        exit(0);
    }
}
