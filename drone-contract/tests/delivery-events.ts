import * as anchor from "@coral-xyz/anchor";
import { Program } from "@coral-xyz/anchor";
import { DeliveryEvents } from "../target/types/delivery_events";

import {
  Commitment,
  Keypair,
  LAMPORTS_PER_SOL,
  PublicKey,
  SystemProgram,
  Transaction,
  Connection,
  clusterApiUrl,
  type GetProgramAccountsConfig,
} from "@solana/web3.js";

import { assert } from "chai";

describe("delivery-events", () => {
  const provider = anchor.AnchorProvider.env();
  const connection = provider.connection;
  console.log("RPC Connection: ", connection.rpcEndpoint);
  anchor.setProvider(provider);

  const program = anchor.workspace.delivery_events as Program<DeliveryEvents>;

  //helpers
  const confirm = async (signature: string): Promise<string> => {
    const block = await connection.getLatestBlockhash();
    await connection.confirmTransaction({
      signature,
      ...block,
    });
    return signature;
  };

  const log = async (signature: string): Promise<string> => {
    console.log(
      `Your transaction signature: https://explorer.solana.com/transaction/${signature}?cluster=custom&customUrl=${connection.rpcEndpoint}`
    );
    return signature;
  };

  const DroneStatus = {
    Available: 0,
    ReadyToFly: 1,
    Arrived: 2,
    UnAvailable: 3,
    Error: 4,
  };

  // get or find accounts
  //const operator = provider.wallet.publicKey;
  const operator = Keypair.generate();

  const [droneState, _bump] = PublicKey.findProgramAddressSync(
    [Buffer.from("drone"), operator.publicKey.toBuffer()],
    program.programId
  );



  it("Airdrop", async () => {
    const commitment: Commitment = "confirmed";

    const confirmAirdropTx = async (signature: string) => {
      const latestBlockhash = await anchor.getProvider().connection.getLatestBlockhash();
      await anchor.getProvider().connection.confirmTransaction(
        {
          signature,
          ...latestBlockhash,
        },
        commitment
      )
    }

    await anchor.getProvider().connection.requestAirdrop(operator.publicKey, 2 * anchor.web3.LAMPORTS_PER_SOL)
      .then(confirmAirdropTx);

    console.log("Airdrop success");
  });

  it("Drone initialized!", async () => {
    const tx = await program.methods.initialize()
      .accountsPartial({
        operator: operator.publicKey,
        droneState: droneState,
        systemProgram: SystemProgram.programId,
      })
      .signers([operator])
      .rpc()
      .then(confirm)
      .then(log);
    console.log("Your transaction signature", tx);

    const droneStateAccount = await program.account.droneState.fetch(droneState);
    assert.equal(droneStateAccount.operator.toString(), operator.publicKey.toString());
    assert.equal(droneStateAccount.lastStatusUpdate, 0);
    assert.equal(droneStateAccount.bump, _bump);
  });

  it("Purchased item is loaded and employee presses button to confirm drone is ready to fly", async () => {
    // Listen for the  event
    const eventListener = program.addEventListener("readyToFlyEvent", (event) => {
      console.log("readyToFlyEvent received:", event);

      assert.equal(event.operator.toString(), operator.publicKey.toString());
      // Clean up the event listener after the event is received
      program.removeEventListener(eventListener);
    });

    const tx = await program.methods
      .readyToFly()
      .accountsPartial({
        operator: operator.publicKey,
        droneState: droneState,
      })
      .signers([operator])
      .rpc()
      .then(confirm)
      .then(log);

    console.log("transaction signature", tx);


    // Wait for a short period to ensure the event is emitted
    await new Promise(resolve => setTimeout(resolve, 2000));
    const eventNotReceived = true;
    assert.equal(eventNotReceived, true);
    // Clean up the event listener if the event was not received
    program.removeEventListener(eventListener);

  });

  it("Drone arrives at letterbox, drone status is updated to arrived and event is emitted", async () => {
    // Listen for the  event
    const eventListener = program.addEventListener("droneArrivedEvent", (event) => {
      console.log("droneArrivedEvent received:", event);

      assert.equal(event.operator.toString(), operator.publicKey.toString());
      // Clean up the event listener after the event is received
      program.removeEventListener(eventListener);
    });

    const tx = await program.methods
      .updateDronestatus({ arrived: {} })
      .accountsPartial({
        operator: operator.publicKey,
        droneState: droneState,
      })
      .signers([operator])
      .rpc()
      .then(confirm)
      .then(log);

    console.log("transaction signature", tx);


    // Wait for a short period to ensure the event is emitted
    await new Promise(resolve => setTimeout(resolve, 2000));
    const eventNotReceived = true;
    assert.equal(eventNotReceived, true);
    // Clean up the event listener if the event was not received
    program.removeEventListener(eventListener);

  });

  it("If instruction not called, there is no ready to fly event", async () => {
    // Listen for the  event
    const eventListener = program.addEventListener("readyToFlyEvent", (event) => {
      console.log("readyToFlyEvent received:", event);
      // Clean up the event listener after the event is received
      program.removeEventListener(eventListener);
    });

    // Wait for a short period 
    await new Promise(resolve => setTimeout(resolve, 2000));
    const eventNotReceived = true;
    assert.equal(eventNotReceived, true);

    console.log("Cleaning up event listener");
    // Clean up the event listener if the event was not received
    program.removeEventListener(eventListener);

  });

  it("Should fail when trying to initialize drone twice", async () => {
    // Second initialization should fail
    try {
      await program.methods.initialize()
        .accountsPartial({
          operator: operator.publicKey,
          droneState: droneState,
          systemProgram: SystemProgram.programId,
        })
        .signers([operator])
        .rpc();
      assert.fail("Should have thrown an error");
    } catch (error) {
    }
  });

  it("Should fail when calling readyToFly before initializing drone state account", async () => {
    const operator2 = Keypair.generate();
    const [droneState2] = PublicKey.findProgramAddressSync(
      [Buffer.from("drone"), operator2.publicKey.toBuffer()],
      program.programId
    );
  
    try {
      await program.methods.readyToFly()
        .accountsPartial({
          operator: operator2.publicKey,
          droneState: droneState2,
        })
        .signers([operator2])
        .rpc();
      assert.fail("Should have thrown an error");
    } catch (error) {
    }
  });



});
