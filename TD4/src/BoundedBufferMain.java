public class BoundedBufferMain {

    public static void main(String[] args) {
        BoundedBuffer buffer;

        // Check the arguments of the command line
        if (args.length != 1) {
            System.out.println("PROGRAM FILENAME");
            System.exit(1);
        }
        Utils.init(args[0]);

        Consumer consumers[] = new Consumer[(int) Utils.nConsumers];
        Producer producers[] = new Producer[(int) Utils.nProducers];

        // Create a buffer
        if (Utils.sem_impl == 0)
            buffer = new NatBoundedBuffer(Utils.bufferSize);
        else
            buffer = new SemBoundedBuffer(Utils.bufferSize);

        // Create producers and then consumers
        // Create producers
        for (int i = 0; i < Utils.nProducers; i++) {
            producers[i] = new Producer(i, buffer);
            producers[i].start();
        }

        // Create consumers
        for (int i = 0; i < Utils.nConsumers; i++) {
            consumers[i] = new Consumer(i, buffer);
            consumers[i].start();
        }

        // Wait for the end of the producers
        for (int i = 0; i < Utils.nProducers; i++) {
            try {
                producers[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        // Wait for the end of the consumers
        for (int i = 0; i < Utils.nConsumers; i++) {
            try {
                consumers[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
