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
    const buyer = Keypair.generate();
    const seller = Keypair.generate();

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
      await anchor.getProvider().connection.requestAirdrop(buyer.publicKey, 2 * anchor.web3.LAMPORTS_PER_SOL)
        .then(confirmAirdropTx);
      await anchor.getProvider().connection.requestAirdrop(seller.publicKey, 2 * anchor.web3.LAMPORTS_PER_SOL)
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

  it("Pharmacy lists medicine for purchase for drone delivery", async () => {
    let product_name = "Cough medicine";
    let product_description = "Relieves cough symptoms";
    let product_price = 10;//USDT ot USDC
    const tx = await program.methods.listProduct(product_name, product_description, product_price)
    .accountsPartial({
      seller: admin.publicKey,
    })
    .signers([admin])
      .rpc()
      .then(confirm)
      .then(log);
    console.log("Your transaction signature", tx);
  });

  xit("Buyer purchases medicine for drone delivery", async () => {
    let product_name = "Cough medicine";
    let quantity = 1;
    const tx = await program.methods.purchaseProduct(product_name, quantity)
    .accountsPartial({
      buyer: buyer.publicKey,
    })
    .signers([buyer])
    .rpc()
      .then(confirm)
      .then(log);
    console.log("Your transaction signature", tx);
  });

  it("When order is placed, event is emitted to notify seller and to drone operators to accept the order", async () => {
    let product_name = "Cough medicine";
    let quantity = 1;

    const [product, _bump] = PublicKey.findProgramAddressSync(
      [Buffer.from("product"), Buffer.from(product_name)],
      program.programId
    );
    const eventListener = program.addEventListener("orderPlacedEvent", (event) => {
      console.log("orderPlacedEvent received:", event);
      assert.equal(event.buyer.toString(), buyer.publicKey.toString());
      assert.equal(event.product.toString(), product_name);
      assert.equal(event.quantity, 1);
      assert.equal(event.price, 10);
    });

    
    const tx = await program.methods.purchaseProduct(product_name, quantity)
    .accountsPartial({
      buyer: buyer.publicKey,
    })
    .signers([buyer])
    .rpc()
    .then(confirm)
    .then(log);
    console.log("Your transaction signature", tx);

  });

});
