import * as anchor from "@coral-xyz/anchor";
import { Program } from "@coral-xyz/anchor";
import { AnchorDroneDeliveryMarketplace } from "../target/types/anchor_drone_delivery_marketplace";
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




describe("anchor-drone-delivery-marketplace", () => {
  // Configure the client to use the local cluster.
  const provider = anchor.AnchorProvider.env();
  const connection = provider.connection;
  anchor.setProvider(provider);

  const program = anchor.workspace.anchorDroneDeliveryMarketplace as Program<AnchorDroneDeliveryMarketplace>;

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
    const admin = Keypair.generate();

    let marketplace_name = "Drone Delivery Marketplace";

    const [marketplace, _bump] = PublicKey.findProgramAddressSync(
      [Buffer.from("marketplace"), Buffer.from(marketplace_name)],
      program.programId
    );

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

    it("Airdrop", async () => {
      await anchor.getProvider().connection.requestAirdrop(admin.publicKey, 2 * anchor.web3.LAMPORTS_PER_SOL)
        .then(confirmAirdropTx);
    });

  it("Marketplace initialized!", async () => {
    let fee = 0.1;
    const tx = await program.methods.initialize(marketplace_name, fee)
    .accountsPartial({
      admin: admin.publicKey,
      marketplace,
      systemProgram: SystemProgram.programId,
    })
    .signers([admin])
      .rpc()
      .then(confirm)
      .then(log);
    console.log("Your transaction signature", tx);
  });


});
