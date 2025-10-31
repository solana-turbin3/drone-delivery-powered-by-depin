use anchor_lang::prelude::*;

use crate::state::{Product, OrderInfo};
use crate::events::OrderPlacedEvent;

#[derive(Accounts)]
#[instruction(product_name: String)]
pub struct PurchaseProduct<'info> {
    #[account(mut)]
    pub buyer: Signer<'info>,

    #[account(
        mut,
        seeds = [b"product", product_name.as_bytes()],
        bump,
    )]
    pub product: Account<'info, Product>,

    #[account(
        init,
        payer = buyer,
        seeds = [b"order", buyer.key().as_ref(),],
        bump,
        space = 8 + OrderInfo::INIT_SPACE
    )]
    pub order_info: Account<'info, OrderInfo>,
   
    pub system_program: Program<'info, System>,

}

impl <'info> PurchaseProduct<'info> {
    pub fn purchase_product(&mut self, product_name: String, quantity: u32, bumps: &PurchaseProductBumps) -> Result<()> {
        assert!(self.product.quantity > 0);

        self.order_info.set_inner(OrderInfo{
            buyer: self.buyer.key(),
            product: self.product.key(),
            quantity: quantity,
            price: self.product.price,
            timestamp: Clock::get()?.unix_timestamp,
            bump: bumps.order_info,
        });

        emit!(OrderPlacedEvent {
            buyer: self.buyer.key(),
            product: self.product.key(),
            quantity: quantity,
            price: self.product.price,
            timestamp: Clock::get()?.unix_timestamp,
        });

        msg!("You purchased a product named: {}", self.product.name);
        
        Ok(())
    }
}