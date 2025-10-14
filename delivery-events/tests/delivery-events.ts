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
  // Configure the client to use the local cluster.
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

  // get or find accounts
  //const admin = provider.wallet.publicKey;
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
  });

  it("Purchased item is loaded and employee presses button to confirm drone is ready to fly", async () => {
    // Listen for the  event
    const eventListener = program.addEventListener("readyToFlyEvent", (event) => {
      console.log("readyToFlyEvent received:", event);
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


    //    assert.equal( );




    // Wait for a short period to ensure the event is emitted
    //await new Promise(resolve => setTimeout(resolve, 5000));

    // Clean up the event listener if the event was not received
    program.removeEventListener(eventListener);

  });



});
